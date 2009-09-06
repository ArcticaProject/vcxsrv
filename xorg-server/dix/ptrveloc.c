/*
 *
 * Copyright © 2006-2009 Simon Thum             simon dot thum at gmx dot de
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include <math.h>
#include <ptrveloc.h>
#include <exevents.h>
#include <X11/Xatom.h>

#include <xserver-properties.h>

/*****************************************************************************
 * Predictable pointer acceleration
 *
 * 2006-2009 by Simon Thum (simon [dot] thum [at] gmx de)
 *
 * Serves 3 complementary functions:
 * 1) provide a sophisticated ballistic velocity estimate to improve
 *    the relation between velocity (of the device) and acceleration
 * 2) make arbitrary acceleration profiles possible
 * 3) decelerate by two means (constant and adaptive) if enabled
 *
 * Important concepts are the
 *
 * - Scheme
 *      which selects the basic algorithm
 *      (see devices.c/InitPointerAccelerationScheme)
 * - Profile
 *      which returns an acceleration
 *      for a given velocity
 *
 *  The profile can be selected by the user at runtime.
 *  The classic profile is intended to cleanly perform old-style
 *  function selection (threshold =/!= 0)
 *
 ****************************************************************************/

/* fwds */
int
SetAccelerationProfile(DeviceVelocityPtr vel, int profile_num);
static float
SimpleSmoothProfile(DeviceIntPtr dev, DeviceVelocityPtr vel, float velocity,
                    float threshold, float acc);
static PointerAccelerationProfileFunc
GetAccelerationProfile(DeviceVelocityPtr vel, int profile_num);

/*#define PTRACCEL_DEBUGGING*/

#ifdef PTRACCEL_DEBUGGING
#define DebugAccelF ErrorF
#else
#define DebugAccelF(...) /* */
#endif

/********************************
 *  Init/Uninit
 *******************************/

/* some int which is not a profile number */
#define PROFILE_UNINITIALIZE (-100)

/**
 * Init struct so it should match the average case
 */
void
InitVelocityData(DeviceVelocityPtr vel)
{
    memset(vel, 0, sizeof(DeviceVelocityRec));

    vel->corr_mul = 10.0;      /* dots per 10 milisecond should be usable */
    vel->const_acceleration = 1.0;   /* no acceleration/deceleration  */
    vel->reset_time = 300;
    vel->use_softening = 1;
    vel->min_acceleration = 1.0; /* don't decelerate */
    vel->max_rel_diff = 0.2;
    vel->max_diff = 1.0;
    vel->initial_range = 2;
    vel->average_accel = TRUE;
    SetAccelerationProfile(vel, AccelProfileClassic);
    InitTrackers(vel, 16);
}


/**
 * Clean up
 */
void
FreeVelocityData(DeviceVelocityPtr vel){
    xfree(vel->tracker);
    SetAccelerationProfile(vel, PROFILE_UNINITIALIZE);
}


/*
 *  dix uninit helper, called through scheme
 */
void
AccelerationDefaultCleanup(DeviceIntPtr dev)
{
    /*sanity check*/
    if( dev->valuator->accelScheme.AccelSchemeProc == acceleratePointerPredictable
            && dev->valuator->accelScheme.accelData != NULL){
        dev->valuator->accelScheme.AccelSchemeProc = NULL;
        FreeVelocityData(dev->valuator->accelScheme.accelData);
        xfree(dev->valuator->accelScheme.accelData);
        dev->valuator->accelScheme.accelData = NULL;
    }
}


/*************************
 * Input property support
 ************************/

/**
 * choose profile
 */
static int
AccelSetProfileProperty(DeviceIntPtr dev, Atom atom,
                        XIPropertyValuePtr val, BOOL checkOnly)
{
    DeviceVelocityPtr vel;
    int profile, *ptr = &profile;
    int rc;
    int nelem = 1;

    if (atom != XIGetKnownProperty(ACCEL_PROP_PROFILE_NUMBER))
        return Success;

    vel = GetDevicePredictableAccelData(dev);
    if (!vel)
        return BadValue;
    rc = XIPropToInt(val, &nelem, &ptr);

    if(checkOnly)
    {
        if (rc)
            return rc;

        if (GetAccelerationProfile(vel, profile) == NULL)
            return BadValue;
    } else
	SetAccelerationProfile(vel, profile);

    return Success;
}

static void
AccelInitProfileProperty(DeviceIntPtr dev, DeviceVelocityPtr vel)
{
    int profile = vel->statistics.profile_number;
    Atom prop_profile_number = XIGetKnownProperty(ACCEL_PROP_PROFILE_NUMBER);

    XIChangeDeviceProperty(dev, prop_profile_number, XA_INTEGER, 32,
                           PropModeReplace, 1, &profile, FALSE);
    XISetDevicePropertyDeletable(dev, prop_profile_number, FALSE);
    XIRegisterPropertyHandler(dev, AccelSetProfileProperty, NULL, NULL);
}

/**
 * constant deceleration
 */
static int
AccelSetDecelProperty(DeviceIntPtr dev, Atom atom,
                      XIPropertyValuePtr val, BOOL checkOnly)
{
    DeviceVelocityPtr vel;
    float v, *ptr = &v;
    int rc;
    int nelem = 1;

    if (atom != XIGetKnownProperty(ACCEL_PROP_CONSTANT_DECELERATION))
        return Success;

    vel = GetDevicePredictableAccelData(dev);
    if (!vel)
        return BadValue;
    rc = XIPropToFloat(val, &nelem, &ptr);

    if(checkOnly)
    {
        if (rc)
            return rc;
	return (v >= 1.0f) ? Success : BadValue;
    }

    if(v >= 1.0f)
	vel->const_acceleration = 1/v;

    return Success;
}

static void
AccelInitDecelProperty(DeviceIntPtr dev, DeviceVelocityPtr vel)
{
    float fval = 1.0/vel->const_acceleration;
    Atom prop_const_decel = XIGetKnownProperty(ACCEL_PROP_CONSTANT_DECELERATION);
    XIChangeDeviceProperty(dev, prop_const_decel,
                           XIGetKnownProperty(XATOM_FLOAT), 32,
                           PropModeReplace, 1, &fval, FALSE);
    XISetDevicePropertyDeletable(dev, prop_const_decel, FALSE);
    XIRegisterPropertyHandler(dev, AccelSetDecelProperty, NULL, NULL);
}


/**
 * adaptive deceleration
 */
static int
AccelSetAdaptDecelProperty(DeviceIntPtr dev, Atom atom,
                           XIPropertyValuePtr val, BOOL checkOnly)
{
    DeviceVelocityPtr veloc;
    float v, *ptr = &v;
    int rc;
    int nelem = 1;

    if (atom != XIGetKnownProperty(ACCEL_PROP_ADAPTIVE_DECELERATION))
        return Success;

    veloc = GetDevicePredictableAccelData(dev);
    if (!veloc)
        return BadValue;
    rc = XIPropToFloat(val, &nelem, &ptr);

    if(checkOnly)
    {
        if (rc)
            return rc;
	return (v >= 1.0f) ? Success : BadValue;
    }

    if(v >= 1.0f)
	veloc->min_acceleration = 1/v;

    return Success;
}

static void
AccelInitAdaptDecelProperty(DeviceIntPtr dev, DeviceVelocityPtr vel)
{
    float fval = 1.0/vel->min_acceleration;
    Atom prop_adapt_decel = XIGetKnownProperty(ACCEL_PROP_ADAPTIVE_DECELERATION);

    XIChangeDeviceProperty(dev, prop_adapt_decel, XIGetKnownProperty(XATOM_FLOAT), 32,
                           PropModeReplace, 1, &fval, FALSE);
    XISetDevicePropertyDeletable(dev, prop_adapt_decel, FALSE);
    XIRegisterPropertyHandler(dev, AccelSetAdaptDecelProperty, NULL, NULL);
}


/**
 * velocity scaling
 */
static int
AccelSetScaleProperty(DeviceIntPtr dev, Atom atom,
                      XIPropertyValuePtr val, BOOL checkOnly)
{
    DeviceVelocityPtr vel;
    float v, *ptr = &v;
    int rc;
    int nelem = 1;

    if (atom != XIGetKnownProperty(ACCEL_PROP_VELOCITY_SCALING))
        return Success;

    vel = GetDevicePredictableAccelData(dev);
    if (!vel)
        return BadValue;
    rc = XIPropToFloat(val, &nelem, &ptr);

    if (checkOnly)
    {
        if (rc)
            return rc;

        return (v > 0) ? Success : BadValue;
    }

    if(v > 0)
	vel->corr_mul = v;

    return Success;
}

static void
AccelInitScaleProperty(DeviceIntPtr dev, DeviceVelocityPtr vel)
{
    float fval = vel->corr_mul;
    Atom prop_velo_scale = XIGetKnownProperty(ACCEL_PROP_VELOCITY_SCALING);

    XIChangeDeviceProperty(dev, prop_velo_scale, XIGetKnownProperty(XATOM_FLOAT), 32,
                           PropModeReplace, 1, &fval, FALSE);
    XISetDevicePropertyDeletable(dev, prop_velo_scale, FALSE);
    XIRegisterPropertyHandler(dev, AccelSetScaleProperty, NULL, NULL);
}

BOOL
InitializePredictableAccelerationProperties(DeviceIntPtr dev)
{
    DeviceVelocityPtr  vel = GetDevicePredictableAccelData(dev);

    if(!vel)
	return FALSE;

    AccelInitProfileProperty(dev, vel);
    AccelInitDecelProperty(dev, vel);
    AccelInitAdaptDecelProperty(dev, vel);
    AccelInitScaleProperty(dev, vel);
    return TRUE;
}

/*********************
 * Tracking logic
 ********************/

void
InitTrackers(DeviceVelocityPtr vel, int ntracker)
{
    if(ntracker < 1){
	ErrorF("(dix ptracc) invalid number of trackers\n");
	return;
    }
    xfree(vel->tracker);
    vel->tracker = (MotionTrackerPtr)xalloc(ntracker * sizeof(MotionTracker));
    memset(vel->tracker, 0, ntracker * sizeof(MotionTracker));
    vel->num_tracker = ntracker;
}

/**
 * return a bit field of possible directions.
 * 0 = N, 2 = E, 4 = S, 6 = W, in-between is as you guess.
 * There's no reason against widening to more precise directions (<45 degrees),
 * should it not perform well. All this is needed for is sort out non-linear
 * motion, so precision isn't paramount. However, one should not flag direction
 * too narrow, since it would then cut the linear segment to zero size way too
 * often.
 */
static int
DoGetDirection(int dx, int dy){
    float r;
    int i1, i2;
    /* on insignificant mickeys, flag 135 degrees */
    if(abs(dx) < 2 && abs(dy < 2)){
	/* first check diagonal cases */
	if(dx > 0 && dy > 0)
	    return 4+8+16;
	if(dx > 0 && dy < 0)
	    return 1+2+4;
	if(dx < 0 && dy < 0)
	    return 1+128+64;
	if(dx < 0 && dy > 0)
	    return 16+32+64;
        /* check axis-aligned directions */
	if(dx > 0)
            return 2+4+8; /*E*/
        if(dx < 0)
            return 128+64+32; /*W*/
        if(dy > 0)
            return 32+16+8; /*S*/
        if(dy < 0)
            return 128+1+2; /*N*/
        return 255; /* shouldn't happen */
    }
    /* else, compute angle and set appropriate flags */
#ifdef _ISOC99_SOURCE
    r = atan2f(dy, dx);
#else
    r = atan2(dy, dx);
#endif
    /* find direction. We avoid r to become negative,
     * since C has no well-defined modulo for such cases. */
    r = (r+(M_PI*2.5))/(M_PI/4);
    /* this intends to flag 2 directions (90 degrees),
     * except on very well-aligned mickeys. */
    i1 = (int)(r+0.1) % 8;
    i2 = (int)(r+0.9) % 8;
    if(i1 < 0 || i1 > 7 || i2 < 0 || i2 > 7)
	return 255; /* shouldn't happen */
    return 1 << i1 | 1 << i2;
}

#define DIRECTION_CACHE_RANGE 5
#define DIRECTION_CACHE_SIZE (DIRECTION_CACHE_RANGE*2+1)

/* cache DoGetDirection(). */
static int
GetDirection(int dx, int dy){
    static int cache[DIRECTION_CACHE_SIZE][DIRECTION_CACHE_SIZE];
    int i;
    if (abs(dx) <= DIRECTION_CACHE_RANGE &&
	abs(dy) <= DIRECTION_CACHE_RANGE) {
	/* cacheable */
	i = cache[DIRECTION_CACHE_RANGE+dx][DIRECTION_CACHE_RANGE+dy];
	if(i != 0){
	    return i;
	}else{
	    i = DoGetDirection(dx, dy);
	    cache[DIRECTION_CACHE_RANGE+dx][DIRECTION_CACHE_RANGE+dy] = i;
	    return i;
	}
    }else{
	/* non-cacheable */
	return DoGetDirection(dx, dy);
    }
}

#undef DIRECTION_CACHE_RANGE
#undef DIRECTION_CACHE_SIZE


/* convert offset (age) to array index */
#define TRACKER_INDEX(s, d) (((s)->num_tracker + (s)->cur_tracker - (d)) % (s)->num_tracker)

static inline void
FeedTrackers(DeviceVelocityPtr vel, int dx, int dy, int cur_t)
{
    int n;
    for(n = 0; n < vel->num_tracker; n++){
	vel->tracker[n].dx += dx;
	vel->tracker[n].dy += dy;
    }
    n = (vel->cur_tracker + 1) % vel->num_tracker;
    vel->tracker[n].dx = 0;
    vel->tracker[n].dy = 0;
    vel->tracker[n].time = cur_t;
    vel->tracker[n].dir = GetDirection(dx, dy);
    DebugAccelF("(dix prtacc) motion [dx: %i dy: %i dir:%i diff: %i]\n",
                dx, dy, vel->tracker[n].dir,
                cur_t - vel->tracker[vel->cur_tracker].time);
    vel->cur_tracker = n;
}

/**
 * calc velocity for given tracker, with
 * velocity scaling.
 * This assumes linear motion.
 */
static float
CalcTracker(DeviceVelocityPtr vel, int offset, int cur_t){
    int index = TRACKER_INDEX(vel, offset);
    float dist = sqrt(  vel->tracker[index].dx * vel->tracker[index].dx
                      + vel->tracker[index].dy * vel->tracker[index].dy);
    int dtime = cur_t - vel->tracker[index].time;
    if(dtime > 0)
	return (dist / dtime);
    else
	return 0;/* synonymous for NaN, since we're not C99 */
}

/* find the most plausible velocity. That is, the most distant
 * (in time) tracker which isn't too old, beyond a linear partition,
 * or simply too much off initial velocity.
 *
 * May return 0.
 */
static float
QueryTrackers(DeviceVelocityPtr vel, int cur_t){
    int n, offset, dir = 255, i = -1, age_ms;
    /* initial velocity: a low-offset, valid velocity */
    float iveloc = 0, res = 0, tmp, vdiff;
    float vfac =  vel->corr_mul * vel->const_acceleration; /* premultiply */
    /* loop from current to older data */
    for(offset = 1; offset < vel->num_tracker; offset++){
	n = TRACKER_INDEX(vel, offset);

	age_ms = cur_t - vel->tracker[n].time;

	/* bail out if data is too old and protect from overrun */
	if (age_ms >= vel->reset_time || age_ms < 0) {
	    DebugAccelF("(dix prtacc) query: tracker too old\n");
	    break;
	}

	/*
	 * this heuristic avoids using the linear-motion velocity formula
	 * in CalcTracker() on motion that isn't exactly linear. So to get
	 * even more precision we could subdivide as a final step, so possible
	 * non-linearities are accounted for.
	 */
	dir &= vel->tracker[n].dir;
	if(dir == 0){
	    DebugAccelF("(dix prtacc) query: no longer linear\n");
	    /* instead of breaking it we might also inspect the partition after,
	     * but actual improvement with this is probably rare. */
	    break;
	}

	tmp = CalcTracker(vel, offset, cur_t) * vfac;

	if ((iveloc == 0 || offset <= vel->initial_range) && tmp != 0) {
	    /* set initial velocity and result */
	    res = iveloc = tmp;
	    i = offset;
	} else if (iveloc != 0 && tmp != 0) {
	    vdiff = fabs(iveloc - tmp);
	    if (vdiff <= vel->max_diff ||
		vdiff/(iveloc + tmp) < vel->max_rel_diff) {
		/* we're in range with the initial velocity,
		 * so this result is likely better
		 * (it contains more information). */
		res = tmp;
		i = offset;
	    }else{
		/* we're not in range, quit - it won't get better. */
		DebugAccelF("(dix prtacc) query: tracker too different:"
		            " old %2.2f initial %2.2f diff: %2.2f\n",
		            tmp, iveloc, vdiff);
		break;
	    }
	}
    }
    if(offset == vel->num_tracker){
	DebugAccelF("(dix prtacc) query: last tracker in effect\n");
	i = vel->num_tracker-1;
    }
    if(i>=0){
        n = TRACKER_INDEX(vel, i);
	DebugAccelF("(dix prtacc) result: offset %i [dx: %i dy: %i diff: %i]\n",
	            i,
	            vel->tracker[n].dx,
	            vel->tracker[n].dy,
	            cur_t - vel->tracker[n].time);
    }
    return res;
}

#undef TRACKER_INDEX

/**
 * Perform velocity approximation based on 2D 'mickeys' (mouse motion delta).
 * return true if non-visible state reset is suggested
 */
short
ProcessVelocityData2D(
    DeviceVelocityPtr vel,
    int dx,
    int dy,
    int time)
{
    float velocity;

    vel->last_velocity = vel->velocity;

    FeedTrackers(vel, dx, dy, time);

    velocity = QueryTrackers(vel, time);

    vel->velocity = velocity;
    return velocity == 0;
}

/**
 * this flattens significant ( > 1) mickeys a little bit for more steady
 * constant-velocity response
 */
static inline float
ApplySimpleSoftening(int od, int d)
{
    float res = d;
    if (d <= 1 && d >= -1)
        return res;
    if (d > od)
        res -= 0.5;
    else if (d < od)
        res += 0.5;
    return res;
}


static void
ApplySofteningAndConstantDeceleration(
        DeviceVelocityPtr vel,
        int dx,
        int dy,
        float* fdx,
        float* fdy,
        short do_soften)
{
    if (do_soften && vel->use_softening) {
        *fdx = ApplySimpleSoftening(vel->last_dx, dx);
        *fdy = ApplySimpleSoftening(vel->last_dy, dy);
    } else {
        *fdx = dx;
        *fdy = dy;
    }

    *fdx *= vel->const_acceleration;
    *fdy *= vel->const_acceleration;
}

/*
 * compute the acceleration for given velocity and enforce min_acceleartion
 */
float
BasicComputeAcceleration(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc){

    float result;
    result = vel->Profile(dev, vel, velocity, threshold, acc);

    /* enforce min_acceleration */
    if (result < vel->min_acceleration)
	result = vel->min_acceleration;
    return result;
}

/**
 * Compute acceleration. Takes into account averaging, nv-reset, etc.
 */
static float
ComputeAcceleration(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float threshold,
    float acc){
    float res;

    if(vel->velocity <= 0){
	DebugAccelF("(dix ptracc) profile skipped\n");
        /*
         * If we have no idea about device velocity, don't pretend it.
         */
	return 1;
    }

    if(vel->average_accel && vel->velocity != vel->last_velocity){
	/* use simpson's rule to average acceleration between
	 * current and previous velocity.
	 * Though being the more natural choice, it causes a minor delay
	 * in comparison, so it can be disabled. */
	res = BasicComputeAcceleration(
	          dev, vel, vel->velocity, threshold, acc);
	res += BasicComputeAcceleration(
	          dev, vel, vel->last_velocity, threshold, acc);
	res += 4.0f * BasicComputeAcceleration(dev, vel,
	                   (vel->last_velocity + vel->velocity) / 2,
	                   threshold, acc);
	res /= 6.0f;
	DebugAccelF("(dix ptracc) profile average [%.2f ... %.2f] is %.3f\n",
	            vel->velocity, vel->last_velocity, res);
        return res;
    }else{
	res = BasicComputeAcceleration(dev, vel,
	                               vel->velocity, threshold, acc);
	DebugAccelF("(dix ptracc) profile sample [%.2f] is %.3f\n",
               vel->velocity, res);
	return res;
    }
}


/*****************************************
 *  Acceleration functions and profiles
 ****************************************/

/**
 * Polynomial function similar previous one, but with f(1) = 1
 */
static float
PolynomialAccelerationProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float ignored,
    float acc)
{
   return pow(velocity, (acc - 1.0) * 0.5);
}


/**
 * returns acceleration for velocity.
 * This profile selects the two functions like the old scheme did
 */
static float
ClassicProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    if (threshold > 0) {
	return SimpleSmoothProfile (dev,
	                            vel,
	                            velocity,
                                    threshold,
                                    acc);
    } else {
	return PolynomialAccelerationProfile (dev,
	                                      vel,
	                                      velocity,
                                              0,
                                              acc);
    }
}


/**
 * Power profile
 * This has a completely smooth transition curve, i.e. no jumps in the
 * derivatives.
 *
 * This has the expense of overall response dependency on min-acceleration.
 * In effect, min_acceleration mimics const_acceleration in this profile.
 */
static float
PowerProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    float vel_dist;

    acc = (acc-1.0) * 0.1f + 1.0; /* without this, acc of 2 is unuseable */

    if (velocity <= threshold)
        return vel->min_acceleration;
    vel_dist = velocity - threshold;
    return (pow(acc, vel_dist)) * vel->min_acceleration;
}


/**
 * just a smooth function in [0..1] -> [0..1]
 *  - point symmetry at 0.5
 *  - f'(0) = f'(1) = 0
 *  - starts faster than a sinoid
 *  - smoothness C1 (Cinf if you dare to ignore endpoints)
 */
static inline float
CalcPenumbralGradient(float x){
    x *= 2.0f;
    x -= 1.0f;
    return 0.5f + (x * sqrt(1.0f - x*x) + asin(x))/M_PI;
}


/**
 * acceleration function similar to classic accelerated/unaccelerated,
 * but with smooth transition in between (and towards zero for adaptive dec.).
 */
static float
SimpleSmoothProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    if(velocity < 1.0f)
        return CalcPenumbralGradient(0.5 + velocity*0.5) * 2.0f - 1.0f;
    if(threshold < 1.0f)
        threshold = 1.0f;
    if (velocity <= threshold)
        return 1;
    velocity /= threshold;
    if (velocity >= acc)
        return acc;
    else
        return 1.0f + (CalcPenumbralGradient(velocity/acc) * (acc - 1.0f));
}


/**
 * This profile uses the first half of the penumbral gradient as a start
 * and then scales linearly.
 */
static float
SmoothLinearProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    float res, nv;

    if(acc > 1.0f)
        acc -= 1.0f; /*this is so acc = 1 is no acceleration */
    else
        return 1.0f;

    nv = (velocity - threshold) * acc * 0.5f;

    if(nv < 0){
        res = 0;
    }else if(nv < 2){
        res = CalcPenumbralGradient(nv*0.25f)*2.0f;
    }else{
        nv -= 2.0f;
        res = nv * 2.0f / M_PI  /* steepness of gradient at 0.5 */
              + 1.0f; /* gradient crosses 2|1 */
    }
    res += vel->min_acceleration;
    return res;
}


static float
LinearProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    return acc * velocity;
}


static float
NoProfile(
    DeviceIntPtr dev,
    DeviceVelocityPtr vel,
    float velocity,
    float threshold,
    float acc)
{
    return 1.0f;
}

static PointerAccelerationProfileFunc
GetAccelerationProfile(
    DeviceVelocityPtr vel,
    int profile_num)
{
    switch(profile_num){
        case AccelProfileClassic:
            return ClassicProfile;
        case AccelProfileDeviceSpecific:
            return vel->deviceSpecificProfile;
        case AccelProfilePolynomial:
            return PolynomialAccelerationProfile;
        case AccelProfileSmoothLinear:
            return SmoothLinearProfile;
        case AccelProfileSimple:
            return SimpleSmoothProfile;
        case AccelProfilePower:
            return PowerProfile;
        case AccelProfileLinear:
            return LinearProfile;
        case AccelProfileNone:
            return NoProfile;
        default:
            return NULL;
    }
}

/**
 * Set the profile by number.
 * Intended to make profiles exchangeable at runtime.
 * If you created a profile, give it a number here and in the header to
 * make it selectable. In case some profile-specific init is needed, here
 * would be a good place, since FreeVelocityData() also calls this with
 * PROFILE_UNINITIALIZE.
 *
 * returns FALSE if profile number is unavailable, TRUE otherwise.
 */
int
SetAccelerationProfile(
    DeviceVelocityPtr vel,
    int profile_num)
{
    PointerAccelerationProfileFunc profile;
    profile = GetAccelerationProfile(vel, profile_num);

    if(profile == NULL && profile_num != PROFILE_UNINITIALIZE)
	return FALSE;

    if(vel->profile_private != NULL){
        /* Here one could free old profile-private data */
        xfree(vel->profile_private);
        vel->profile_private = NULL;
    }
    /* Here one could init profile-private data */
    vel->Profile = profile;
    vel->statistics.profile_number = profile_num;
    return TRUE;
}

/**********************************************
 * driver interaction
 **********************************************/


/**
 * device-specific profile
 *
 * The device-specific profile is intended as a hook for a driver
 * which may want to provide an own acceleration profile.
 * It should not rely on profile-private data, instead
 * it should do init/uninit in the driver (ie. with DEVICE_INIT and friends).
 * Users may override or choose it.
 */
void
SetDeviceSpecificAccelerationProfile(
        DeviceVelocityPtr vel,
        PointerAccelerationProfileFunc profile)
{
    if(vel)
	vel->deviceSpecificProfile = profile;
}

/**
 * Use this function to obtain a DeviceVelocityPtr for a device. Will return NULL if
 * the predictable acceleration scheme is not in effect.
 */
DeviceVelocityPtr
GetDevicePredictableAccelData(
	DeviceIntPtr dev)
{
    /*sanity check*/
    if(!dev){
	ErrorF("[dix] accel: DeviceIntPtr was NULL");
	return NULL;
    }
    if( dev->valuator &&
	dev->valuator->accelScheme.AccelSchemeProc ==
	    acceleratePointerPredictable &&
	dev->valuator->accelScheme.accelData != NULL){

	return (DeviceVelocityPtr)dev->valuator->accelScheme.accelData;
    }
    return NULL;
}

/********************************
 *  acceleration schemes
 *******************************/

/**
 * Modifies valuators in-place.
 * This version employs a velocity approximation algorithm to
 * enable fine-grained predictable acceleration profiles.
 */
void
acceleratePointerPredictable(
    DeviceIntPtr dev,
    int first_valuator,
    int num_valuators,
    int *valuators,
    int evtime)
{
    float mult = 0.0;
    int dx = 0, dy = 0;
    int *px = NULL, *py = NULL;
    DeviceVelocityPtr velocitydata =
	(DeviceVelocityPtr) dev->valuator->accelScheme.accelData;
    float fdx, fdy, tmp; /* no need to init */
    Bool soften = TRUE;

    if (!num_valuators || !valuators || !velocitydata)
        return;

    if (velocitydata->statistics.profile_number == AccelProfileNone &&
	velocitydata->const_acceleration == 1.0f) {
	return; /*we're inactive anyway, so skip the whole thing.*/
    }

    if (first_valuator == 0) {
        dx = valuators[0];
        px = &valuators[0];
    }
    if (first_valuator <= 1 && num_valuators >= (2 - first_valuator)) {
        dy = valuators[1 - first_valuator];
        py = &valuators[1 - first_valuator];
    }

    if (dx || dy){
        /* reset non-visible state? */
        if (ProcessVelocityData2D(velocitydata, dx , dy, evtime)) {
            soften = FALSE;
        }

        if (dev->ptrfeed && dev->ptrfeed->ctrl.num) {
            /* invoke acceleration profile to determine acceleration */
            mult = ComputeAcceleration (dev, velocitydata,
					dev->ptrfeed->ctrl.threshold,
					(float)dev->ptrfeed->ctrl.num /
					(float)dev->ptrfeed->ctrl.den);

            if(mult != 1.0 || velocitydata->const_acceleration != 1.0) {
                ApplySofteningAndConstantDeceleration( velocitydata,
						       dx, dy,
						       &fdx, &fdy,
						       (mult > 1.0) && soften);

                if (dx) {
                    tmp = mult * fdx + dev->last.remainder[0];
                    /* Since it may not be apparent: lrintf() does not offer
                     * strong statements about rounding; however because we
                     * process each axis conditionally, there's no danger
                     * of a toggling remainder. Its lack of guarantees likely
                     * makes it faster on the average target. */
                    *px = lrintf(tmp);
                    dev->last.remainder[0] = tmp - (float)*px;
                }
                if (dy) {
                    tmp = mult * fdy + dev->last.remainder[1];
                    *py = lrintf(tmp);
                    dev->last.remainder[1] = tmp - (float)*py;
                }
                DebugAccelF("pos (%i | %i) remainders x: %.3f y: %.3f delta x:%.3f y:%.3f\n",
                            *px, *py, dev->last.remainder[0], dev->last.remainder[1], fdx, fdy);
            }
        }
    }
    /* remember last motion delta (for softening/slow movement treatment) */
    velocitydata->last_dx = dx;
    velocitydata->last_dy = dy;
}



/**
 * Originally a part of xf86PostMotionEvent; modifies valuators
 * in-place. Retained mostly for embedded scenarios.
 */
void
acceleratePointerLightweight(
    DeviceIntPtr dev,
    int first_valuator,
    int num_valuators,
    int *valuators,
    int ignored)
{
    float mult = 0.0;
    int dx = 0, dy = 0;
    int *px = NULL, *py = NULL;

    if (!num_valuators || !valuators)
        return;

    if (first_valuator == 0) {
        dx = valuators[0];
        px = &valuators[0];
    }
    if (first_valuator <= 1 && num_valuators >= (2 - first_valuator)) {
        dy = valuators[1 - first_valuator];
        py = &valuators[1 - first_valuator];
    }

    if (!dx && !dy)
        return;

    if (dev->ptrfeed && dev->ptrfeed->ctrl.num) {
        /* modeled from xf86Events.c */
        if (dev->ptrfeed->ctrl.threshold) {
            if ((abs(dx) + abs(dy)) >= dev->ptrfeed->ctrl.threshold) {
                dev->last.remainder[0] = ((float)dx *
                                             (float)(dev->ptrfeed->ctrl.num)) /
                                             (float)(dev->ptrfeed->ctrl.den) +
                                            dev->last.remainder[0];
                if (px) {
                    *px = (int)dev->last.remainder[0];
                    dev->last.remainder[0] = dev->last.remainder[0] -
                                                (float)(*px);
                }

                dev->last.remainder[1] = ((float)dy *
                                             (float)(dev->ptrfeed->ctrl.num)) /
                                             (float)(dev->ptrfeed->ctrl.den) +
                                            dev->last.remainder[1];
                if (py) {
                    *py = (int)dev->last.remainder[1];
                    dev->last.remainder[1] = dev->last.remainder[1] -
                                                (float)(*py);
                }
            }
        }
        else {
	    mult = pow((float)dx * (float)dx + (float)dy * (float)dy,
                       ((float)(dev->ptrfeed->ctrl.num) /
                        (float)(dev->ptrfeed->ctrl.den) - 1.0) /
                       2.0) / 2.0;
            if (dx) {
                dev->last.remainder[0] = mult * (float)dx +
                                            dev->last.remainder[0];
                *px = (int)dev->last.remainder[0];
                dev->last.remainder[0] = dev->last.remainder[0] -
                                            (float)(*px);
            }
            if (dy) {
                dev->last.remainder[1] = mult * (float)dy +
                                            dev->last.remainder[1];
                *py = (int)dev->last.remainder[1];
                dev->last.remainder[1] = dev->last.remainder[1] -
                                            (float)(*py);
            }
        }
    }
}
