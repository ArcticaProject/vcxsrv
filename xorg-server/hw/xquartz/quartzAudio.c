//
// QuartzAudio.m
//
// X Window bell support using CoreAudio or AppKit.
// Greg Parker  gparker@cs.stanford.edu  19 Feb 2001
//
// Info about sine wave sound playback:
// CoreAudio code derived from macosx-dev posting by Tim Wood
//  http://www.omnigroup.com/mailman/archive/macosx-dev/2000-May/002004.html
// Smoothing transitions between sounds
//  http://www.wam.umd.edu/~mphoenix/dss/dss.html
//
/*
 * Copyright (c) 2001 Greg Parker. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE ABOVE LISTED COPYRIGHT HOLDER(S) BE LIABLE FOR ANY CLAIM, DAMAGES OR
 * OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Except as contained in this notice, the name(s) of the above copyright
 * holders shall not be used in advertising or otherwise to promote the sale,
 * use or other dealings in this Software without prior written authorization.
 */

#include "sanitizedCarbon.h"

#ifdef HAVE_DIX_CONFIG_H
#include <dix-config.h>
#endif

#include "quartzCommon.h"
#include "quartzAudio.h"

#include <CoreAudio/CoreAudio.h>
#include <pthread.h>
#include <AvailabilityMacros.h>

#include "inputstr.h"
#include <X11/extensions/XI.h>
#include <assert.h>

void NSBeep(void);

typedef struct QuartzAudioRec {
    double frequency;
    double amplitude;

    UInt32 curFrame;
    UInt32 remainingFrames;
    UInt32 totalFrames;
    double sampleRate;
    UInt32 fadeLength;

    UInt32 bufferByteCount;
    Boolean playing;
    pthread_mutex_t lock;

    // used to fade out interrupted sound and avoid 'pop'
    double prevFrequency;
    double prevAmplitude;
    UInt32 prevFrame; 
} QuartzAudioRec;

static AudioDeviceID quartzAudioDevice = kAudioDeviceUnknown;
static QuartzAudioRec data;


/*
 * QuartzAudioEnvelope
 *  Fade sound in and out to avoid pop.
 *  Sounds with shorter duration will never reach full amplitude. Deal.
 */
static double QuartzAudioEnvelope(
    UInt32 curFrame,
    UInt32 totalFrames,
    UInt32 fadeLength )
{
    double fadeFrames = min(fadeLength, totalFrames / 2);
    if (fadeFrames < 1) return 0;

    if (curFrame < fadeFrames) {
        return curFrame / fadeFrames;
    } else if (curFrame > totalFrames - fadeFrames) {
        return (totalFrames-curFrame) / fadeFrames;
    } else {
        return 1.0;
    }
}


/*
 * QuartzFillBuffer
 *  Fill this buffer with data and update the data position.
 *  FIXME: this is ugly
 */
static void QuartzFillBuffer(
    AudioBuffer *audiobuffer,
    QuartzAudioRec *data )
{
    float *buffer, *b;
    unsigned int frame, frameCount;
    unsigned int bufferFrameCount;
    float multiplier, v;
    int i;
    
    buffer = (float *)audiobuffer->mData;
    bufferFrameCount = audiobuffer->mDataByteSize / (sizeof(float) * audiobuffer->mNumberChannels);

    frameCount = min(bufferFrameCount, data->remainingFrames);

    // Fade out previous sine wave, if any.
    b = buffer;
    if (data->prevFrame) {
        multiplier = 2*M_PI*(data->prevFrequency/data->sampleRate);
        for (frame = 0; frame < data->fadeLength; frame++) {
            v = data->prevAmplitude *
                QuartzAudioEnvelope(frame+data->fadeLength,
                                    2*data->fadeLength,
                                    data->fadeLength) *
                sin(multiplier * (data->prevFrame+frame));
            for (i = 0; i < audiobuffer->mNumberChannels; i++) {
                *b++ = v;
            }
        }
        // no more prev fade
        data->prevFrame = 0;

        // adjust for space eaten by prev fade
        b += audiobuffer->mNumberChannels*frame;
        bufferFrameCount -= frame;
        frameCount = min(bufferFrameCount, data->remainingFrames);
    }

    // Write a sine wave with the specified frequency and amplitude
    multiplier = 2*M_PI*(data->frequency/data->sampleRate);
    for (frame = 0; frame < frameCount; frame++) {
        v = data->amplitude * 
            QuartzAudioEnvelope(data->curFrame+frame, data->totalFrames,
                                data->fadeLength) *
            sin(multiplier * (data->curFrame+frame));
        for (i = 0; i < audiobuffer->mNumberChannels; i++) {
            *b++ = v;
        }
    }

    // Zero out the rest of the buffer, if any
    memset(b, 0, sizeof(float) * audiobuffer->mNumberChannels *
           (bufferFrameCount-frame));

    data->curFrame += frameCount;
    data->remainingFrames -= frameCount;
    if (data->remainingFrames == 0) {
        data->playing = FALSE;
        data->curFrame = 0;
    }
}


/*
 * QuartzAudioIOProc
 *  Callback function for audio playback.
 *  FIXME: use inOutputTime to correct for skipping
 */
static OSStatus 
QuartzAudioIOProc(
    AudioDeviceID inDevice, 
    const AudioTimeStamp *inNow, 
    const AudioBufferList *inInputData, 
    const AudioTimeStamp *inInputTime, 
    AudioBufferList *outOutputData, 
    const AudioTimeStamp *inOutputTime, 
    void *inClientData )
{
    QuartzAudioRec *data = (QuartzAudioRec *)inClientData;
    int i;
    Boolean wasPlaying;

    pthread_mutex_lock(&data->lock);
    wasPlaying = data->playing;
    for (i = 0; i < outOutputData->mNumberBuffers; i++) {
        if (data->playing) {
            QuartzFillBuffer(outOutputData->mBuffers+i, data); 
        }
        else {
            memset(outOutputData->mBuffers[i].mData, 0, 
                   outOutputData->mBuffers[i].mDataByteSize);
        }
    }
    if (wasPlaying  &&  !data->playing) {
        OSStatus err;
        err = AudioDeviceStop(inDevice, QuartzAudioIOProc);
        if(err != noErr)
            fprintf(stderr, "Error stopping audio device: %ld\n", (long int)err);
    }
    pthread_mutex_unlock(&data->lock);
    return 0;
}


/*
 * DDXRingBell
 * Play a tone using the CoreAudio API
 */
void DDXRingBell(
    int volume,         // volume is % of max
    int pitch,          // pitch is Hz
    int duration )      // duration is milliseconds
{
    if (quartzUseSysBeep) {
        if (volume)
            NSBeep();
        return;
    }
        
    if (quartzAudioDevice == kAudioDeviceUnknown) return;

    pthread_mutex_lock(&data.lock);

    // fade previous sound, if any
    data.prevFrequency = data.frequency;
    data.prevAmplitude = data.amplitude;
    data.prevFrame = data.curFrame;

    // set new sound
    data.frequency = pitch;
    data.amplitude = volume / 100.0;
    data.curFrame = 0;
    data.totalFrames = (int)(data.sampleRate * duration / 1000.0);
    data.remainingFrames = data.totalFrames;

    if (! data.playing) {
        OSStatus status;
        status = AudioDeviceStart(quartzAudioDevice, QuartzAudioIOProc);
        if (status) {
            ErrorF("DDXRingBell: AudioDeviceStart returned %ld\n", (long)status);
        } else {
            data.playing = TRUE;
        }
    }
    pthread_mutex_unlock(&data.lock);
}

/*
 * QuartzAudioInit
 *  Prepare to play the bell with the CoreAudio API
 */
void QuartzAudioInit(void) 
{
    UInt32 propertySize;
    OSStatus status;
    AudioDeviceID outputDevice;
    double sampleRate;
    AudioObjectPropertyAddress devicePropertyAddress = { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
    AudioObjectPropertyAddress sampleRatePropertyAddress = { kAudioDevicePropertyNominalSampleRate, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
    
    // Get the default output device
    propertySize = sizeof(outputDevice);
    status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &devicePropertyAddress,
                                        0, NULL,
                                        &propertySize, &outputDevice);
    if (status) {
        ErrorF("QuartzAudioInit: AudioObjectGetPropertyData(output device) returned %ld\n",
               (long)status);
        return;
    }
    if (outputDevice == kAudioDeviceUnknown) {
        ErrorF("QuartzAudioInit: No audio output devices available.\n");
        return;
    }

    // Get the basic device description
    sampleRate = 0.;
    propertySize = sizeof(sampleRate);
    status = AudioObjectGetPropertyData(outputDevice, &sampleRatePropertyAddress,
                                        0, NULL,
                                        &propertySize, &sampleRate);
    if (status) {
        ErrorF("QuartzAudioInit: AudioObjectGetPropertyData(sample rate) returned %ld\n",
               (long)status);
        return;
    }

    // Fill in the playback data
    data.frequency = 0;
    data.amplitude = 0;
    data.curFrame = 0;
    data.remainingFrames = 0; 
    data.sampleRate = sampleRate;
    // data.bufferByteCount = bufferByteCount;
    data.playing = FALSE;
    data.prevAmplitude = 0;
    data.prevFrame = 0;
    data.prevFrequency = 0;
    data.fadeLength = data.sampleRate / 200;
    pthread_mutex_init(&data.lock, NULL); // fixme error check

    // fixme assert fadeLength<framesPerBuffer

    // Prepare for playback
#if MAC_OS_X_VERSION_MIN_REQUIRED >= 1050
    {
    AudioDeviceIOProcID sInputIOProcID = NULL;
    status = AudioDeviceCreateIOProcID( outputDevice, QuartzAudioIOProc, &data, &sInputIOProcID );
    }
#else
    status = AudioDeviceAddIOProc(outputDevice, QuartzAudioIOProc, &data);
#endif
    if (status) {
        ErrorF("QuartzAudioInit: AddIOProc returned %ld\n", (long)status);
        return;
    }

    // success!
    quartzAudioDevice = outputDevice;
}
