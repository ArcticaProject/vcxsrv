#!/usr/bin/env python
from xml.etree.cElementTree import *
from os.path import basename
import getopt
import sys
import re

# Jump to the bottom of this file for the main routine

# Some hacks to make the API more readable, and to keep backwards compability
_cname_re = re.compile('([A-Z0-9][a-z]+|[A-Z0-9]+(?![a-z])|[a-z]+)')
_cname_special_cases = {'DECnet':'decnet'}

_extension_special_cases = ['XPrint', 'XCMisc', 'BigRequests']

_cplusplus_annoyances = {'class' : '_class',
                         'new'   : '_new',
                         'delete': '_delete'}

_hlines = []
_hlevel = 0
_clines = []
_clevel = 0
_ns = None

def _h(fmt, *args):
    '''
    Writes the given line to the header file.
    '''
    _hlines[_hlevel].append(fmt % args)
    
def _c(fmt, *args):
    '''
    Writes the given line to the source file.
    '''
    _clines[_clevel].append(fmt % args)
    
def _hc(fmt, *args):
    '''
    Writes the given line to both the header and source files.
    '''
    _h(fmt, *args)
    _c(fmt, *args)

# XXX See if this level thing is really necessary.
def _h_setlevel(idx):
    '''
    Changes the array that header lines are written to.
    Supports writing different sections of the header file.
    '''
    global _hlevel
    while len(_hlines) <= idx:
        _hlines.append([])
    _hlevel = idx
    
def _c_setlevel(idx):
    '''
    Changes the array that source lines are written to.
    Supports writing to different sections of the source file.
    '''
    global _clevel
    while len(_clines) <= idx:
        _clines.append([])
    _clevel = idx
    
def _n_item(str):
    '''
    Does C-name conversion on a single string fragment.
    Uses a regexp with some hard-coded special cases.
    '''
    if str in _cname_special_cases:
        return _cname_special_cases[str]
    else:
        split = _cname_re.finditer(str)
        name_parts = [match.group(0) for match in split]
        return '_'.join(name_parts)
    
def _cpp(str):
    '''
    Checks for certain C++ reserved words and fixes them.
    '''
    if str in _cplusplus_annoyances:
        return _cplusplus_annoyances[str]
    else:
        return str

def _ext(str):
    '''
    Does C-name conversion on an extension name.
    Has some additional special cases on top of _n_item.
    '''
    if str in _extension_special_cases:
        return _n_item(str).lower()
    else:
        return str.lower()
    
def _n(list):
    '''
    Does C-name conversion on a tuple of strings.
    Different behavior depending on length of tuple, extension/not extension, etc.
    Basically C-name converts the individual pieces, then joins with underscores.
    '''
    if len(list) == 1:
        parts = list
    elif len(list) == 2:
        parts = [list[0], _n_item(list[1])]
    elif _ns.is_ext:
        parts = [list[0], _ext(list[1])] + [_n_item(i) for i in list[2:]]
    else:
        parts = [list[0]] + [_n_item(i) for i in list[1:]]
    return '_'.join(parts).lower()

def _t(list):
    '''
    Does C-name conversion on a tuple of strings representing a type.
    Same as _n but adds a "_t" on the end.
    '''
    if len(list) == 1:
        parts = list
    elif len(list) == 2:
        parts = [list[0], _n_item(list[1]), 't']
    elif _ns.is_ext:
        parts = [list[0], _ext(list[1])] + [_n_item(i) for i in list[2:]] + ['t']
    else:
        parts = [list[0]] + [_n_item(i) for i in list[1:]] + ['t']
    return '_'.join(parts).lower()
        

def c_open(self):
    '''
    Exported function that handles module open.
    Opens the files and writes out the auto-generated comment, header file includes, etc.
    '''
    global _ns
    _ns = self.namespace
    _ns.c_ext_global_name = _n(_ns.prefix + ('id',))

    # Build the type-name collision avoidance table used by c_enum
    build_collision_table()

    _h_setlevel(0)
    _c_setlevel(0)

    _hc('/*')
    _hc(' * This file generated automatically from %s by c_client.py.', _ns.file)
    _hc(' * Edit at your peril.')
    _hc(' */')
    _hc('')

    _h('/**')
    _h(' * @defgroup XCB_%s_API XCB %s API', _ns.ext_name, _ns.ext_name)
    _h(' * @brief %s XCB Protocol Implementation.', _ns.ext_name)
    _h(' * @{')
    _h(' **/')
    _h('')
    _h('#ifndef __%s_H', _ns.header.upper())
    _h('#define __%s_H', _ns.header.upper())
    _h('')
    _h('#include "xcb.h"')

    _c('#include <string.h>')
    _c('#include <assert.h>')
    _c('#include "xcbext.h"')
    _c('#include "%s.h"', _ns.header)
        
    if _ns.is_ext:
        for (n, h) in self.imports:
            _hc('#include "%s.h"', h)

    _h('')
    _h('#ifdef __cplusplus')
    _h('extern "C" {')
    _h('#endif')

    if _ns.is_ext:
        _h('')
        _h('#define XCB_%s_MAJOR_VERSION %s', _ns.ext_name.upper(), _ns.major_version)
        _h('#define XCB_%s_MINOR_VERSION %s', _ns.ext_name.upper(), _ns.minor_version)
        _h('  ') #XXX
        _h('extern xcb_extension_t %s;', _ns.c_ext_global_name)

        _c('')
        _c('xcb_extension_t %s = { "%s", 0 };', _ns.c_ext_global_name, _ns.ext_xname)

def c_close(self):
    '''
    Exported function that handles module close.
    Writes out all the stored content lines, then closes the files.
    '''
    _h_setlevel(2)
    _c_setlevel(2)
    _hc('')

    _h('')
    _h('#ifdef __cplusplus')
    _h('}')
    _h('#endif')

    _h('')
    _h('#endif')
    _h('')
    _h('/**')
    _h(' * @}')
    _h(' */')

    # Write header file
    hfile = open('%s.h' % _ns.header, 'w')
    for list in _hlines:
        for line in list:
            hfile.write(line)
            hfile.write('\n')
    hfile.close()

    # Write source file
    cfile = open('%s.c' % _ns.header, 'w')
    for list in _clines:
        for line in list:
            cfile.write(line)
            cfile.write('\n')
    cfile.close()

def build_collision_table():
    global namecount
    namecount = {}

    for v in module.types.values():
        name = _t(v[0])
        namecount[name] = (namecount.get(name) or 0) + 1

def c_enum(self, name):
    '''
    Exported function that handles enum declarations.
    '''

    tname = _t(name)
    if namecount[tname] > 1:
        tname = _t(name + ('enum',))

    _h_setlevel(0)
    _h('')
    _h('typedef enum %s {', tname)

    count = len(self.values)

    for (enam, eval) in self.values:
        count = count - 1
        equals = ' = ' if eval != '' else ''
        comma = ',' if count > 0 else ''
        _h('    %s%s%s%s', _n(name + (enam,)).upper(), equals, eval, comma)

    _h('} %s;', tname)

def _c_type_setup(self, name, postfix):
    '''
    Sets up all the C-related state by adding additional data fields to
    all Field and Type objects.  Here is where we figure out most of our
    variable and function names.

    Recurses into child fields and list member types.
    '''
    # Do all the various names in advance
    self.c_type = _t(name + postfix)
    self.c_wiretype = 'char' if self.c_type == 'void' else self.c_type

    self.c_iterator_type = _t(name + ('iterator',))
    self.c_next_name = _n(name + ('next',))
    self.c_end_name = _n(name + ('end',))

    self.c_request_name = _n(name)
    self.c_checked_name = _n(name + ('checked',))
    self.c_unchecked_name = _n(name + ('unchecked',))
    self.c_reply_name = _n(name + ('reply',))
    self.c_reply_type = _t(name + ('reply',))
    self.c_cookie_type = _t(name + ('cookie',))

    if self.is_container:

        self.c_container = 'union' if self.is_union else 'struct'
        prev_varsized_field = None
        prev_varsized_offset = 0
        first_field_after_varsized = None

        for field in self.fields:
            _c_type_setup(field.type, field.field_type, ())
            if field.type.is_list:
                _c_type_setup(field.type.member, field.field_type, ())

            field.c_field_type = _t(field.field_type)
            field.c_field_const_type = ('' if field.type.nmemb == 1 else 'const ') + field.c_field_type
            field.c_field_name = _cpp(field.field_name)
            field.c_subscript = '[%d]' % field.type.nmemb if (field.type.nmemb > 1) else ''
            field.c_pointer = ' ' if field.type.nmemb == 1 else '*'

            field.c_iterator_type = _t(field.field_type + ('iterator',))      # xcb_fieldtype_iterator_t
            field.c_iterator_name = _n(name + (field.field_name, 'iterator')) # xcb_container_field_iterator
            field.c_accessor_name = _n(name + (field.field_name,))            # xcb_container_field
            field.c_length_name = _n(name + (field.field_name, 'length'))     # xcb_container_field_length
            field.c_end_name = _n(name + (field.field_name, 'end'))           # xcb_container_field_end

            field.prev_varsized_field = prev_varsized_field
            field.prev_varsized_offset = prev_varsized_offset

            if prev_varsized_offset == 0:
                first_field_after_varsized = field
            field.first_field_after_varsized = first_field_after_varsized

            if field.type.fixed_size():
                prev_varsized_offset += field.type.size
            else:
                self.last_varsized_field = field
                prev_varsized_field = field
                prev_varsized_offset = 0

def _c_iterator_get_end(field, accum):
    '''
    Figures out what C code is needed to find the end of a variable-length structure field.
    For nested structures, recurses into its last variable-sized field.
    For lists, calls the end function
    '''
    if field.type.is_container:
        accum = field.c_accessor_name + '(' + accum + ')'
        # XXX there could be fixed-length fields at the end
        return _c_iterator_get_end(field.type.last_varsized_field, accum)
    if field.type.is_list:
        # XXX we can always use the first way
        if field.type.member.is_simple:
            return field.c_end_name + '(' + accum + ')'
        else:
            return field.type.member.c_end_name + '(' + field.c_iterator_name + '(' + accum + '))'

def _c_iterator(self, name):
    '''
    Declares the iterator structure and next/end functions for a given type.
    '''
    _h_setlevel(0)
    _h('')
    _h('/**')
    _h(' * @brief %s', self.c_iterator_type)
    _h(' **/')
    _h('typedef struct %s {', self.c_iterator_type)
    _h('    %s *data; /**<  */', self.c_type)
    _h('    int%s rem; /**<  */', ' ' * (len(self.c_type) - 2))
    _h('    int%s index; /**<  */', ' ' * (len(self.c_type) - 2))
    _h('} %s;', self.c_iterator_type)

    _h_setlevel(1)
    _c_setlevel(1)
    _h('')
    _h('/**')
    _h(' * Get the next element of the iterator')
    _h(' * @param i Pointer to a %s', self.c_iterator_type)
    _h(' *')
    _h(' * Get the next element in the iterator. The member rem is')
    _h(' * decreased by one. The member data points to the next')
    _h(' * element. The member index is increased by sizeof(%s)', self.c_type)
    _h(' */')
    _c('')
    _hc('')
    _hc('/*****************************************************************************')
    _hc(' **')
    _hc(' ** void %s', self.c_next_name)
    _hc(' ** ')
    _hc(' ** @param %s *i', self.c_iterator_type)
    _hc(' ** @returns void')
    _hc(' **')
    _hc(' *****************************************************************************/')
    _hc(' ')
    _hc('void')
    _h('%s (%s *i  /**< */);', self.c_next_name, self.c_iterator_type)
    _c('%s (%s *i  /**< */)', self.c_next_name, self.c_iterator_type)
    _c('{')

    if not self.fixed_size():
        _c('    %s *R = i->data;', self.c_type)
        _c('    xcb_generic_iterator_t child = %s;', _c_iterator_get_end(self.last_varsized_field, 'R'))
        _c('    --i->rem;')
        _c('    i->data = (%s *) child.data;', self.c_type)
        _c('    i->index = child.index;')
    else:
        _c('    --i->rem;')
        _c('    ++i->data;')
        _c('    i->index += sizeof(%s);', self.c_type)

    _c('}')

    _h('')
    _h('/**')
    _h(' * Return the iterator pointing to the last element')
    _h(' * @param i An %s', self.c_iterator_type)
    _h(' * @return  The iterator pointing to the last element')
    _h(' *')
    _h(' * Set the current element in the iterator to the last element.')
    _h(' * The member rem is set to 0. The member data points to the')
    _h(' * last element.')
    _h(' */')
    _c('')
    _hc('')
    _hc('/*****************************************************************************')
    _hc(' **')
    _hc(' ** xcb_generic_iterator_t %s', self.c_end_name)
    _hc(' ** ')
    _hc(' ** @param %s i', self.c_iterator_type)
    _hc(' ** @returns xcb_generic_iterator_t')
    _hc(' **')
    _hc(' *****************************************************************************/')
    _hc(' ')
    _hc('xcb_generic_iterator_t')
    _h('%s (%s i  /**< */);', self.c_end_name, self.c_iterator_type)
    _c('%s (%s i  /**< */)', self.c_end_name, self.c_iterator_type)
    _c('{')
    _c('    xcb_generic_iterator_t ret;')

    if self.fixed_size():
        _c('    ret.data = i.data + i.rem;')
        _c('    ret.index = i.index + ((char *) ret.data - (char *) i.data);')
        _c('    ret.rem = 0;')
    else:
        _c('    while(i.rem > 0)')
        _c('        %s(&i);', self.c_next_name)
        _c('    ret.data = i.data;')
        _c('    ret.rem = i.rem;')
        _c('    ret.index = i.index;')

    _c('    return ret;')
    _c('}')

def _c_accessor_get_length(expr, prefix=''):
    '''
    Figures out what C code is needed to get a length field.
    For fields that follow a variable-length field, use the accessor.
    Otherwise, just reference the structure field directly.
    '''
    prefarrow = '' if prefix == '' else prefix + '->'

    if expr.lenfield != None and expr.lenfield.prev_varsized_field != None:
        return expr.lenfield.c_accessor_name + '(' + prefix + ')'
    elif expr.lenfield_name != None:
        return prefarrow + expr.lenfield_name
    else:
        return str(expr.nmemb)

def _c_accessor_get_expr(expr, prefix=''):
    '''
    Figures out what C code is needed to get the length of a list field.
    Recurses for math operations.
    Returns bitcount for value-mask fields.
    Otherwise, uses the value of the length field.
    '''
    lenexp = _c_accessor_get_length(expr, prefix)

    if expr.op != None:
        return '(' + _c_accessor_get_expr(expr.lhs, prefix) + ' ' + expr.op + ' ' + _c_accessor_get_expr(expr.rhs, prefix) + ')'
    elif expr.bitfield:
        return 'xcb_popcount(' + lenexp + ')'
    else:
        return lenexp

def _c_accessors_field(self, field):
    '''
    Declares the accessor functions for a non-list field that follows a variable-length field.
    '''
    if field.type.is_simple:
        _hc('')
        _hc('')
        _hc('/*****************************************************************************')
        _hc(' **')
        _hc(' ** %s %s', field.c_field_type, field.c_accessor_name)
        _hc(' ** ')
        _hc(' ** @param const %s *R', self.c_type)
        _hc(' ** @returns %s', field.c_field_type)
        _hc(' **')
        _hc(' *****************************************************************************/')
        _hc(' ')
        _hc('%s', field.c_field_type)
        _h('%s (const %s *R  /**< */);', field.c_accessor_name, self.c_type)
        _c('%s (const %s *R  /**< */)', field.c_accessor_name, self.c_type)
        _c('{')
        _c('    xcb_generic_iterator_t prev = %s;', _c_iterator_get_end(field.prev_varsized_field, 'R'))
        _c('    return * (%s *) ((char *) prev.data + XCB_TYPE_PAD(%s, prev.index) + %d);', field.c_field_type, field.first_field_after_varsized.type.c_type, field.prev_varsized_offset)
        _c('}')
    else:
        _hc('')
        _hc('')
        _hc('/*****************************************************************************')
        _hc(' **')
        _hc(' ** %s * %s', field.c_field_type, field.c_accessor_name)
        _hc(' ** ')
        _hc(' ** @param const %s *R', self.c_type)
        _hc(' ** @returns %s *', field.c_field_type)
        _hc(' **')
        _hc(' *****************************************************************************/')
        _hc(' ')
        _hc('%s *', field.c_field_type)
        _h('%s (const %s *R  /**< */);', field.c_accessor_name, self.c_type)
        _c('%s (const %s *R  /**< */)', field.c_accessor_name, self.c_type)
        _c('{')
        _c('    xcb_generic_iterator_t prev = %s;', _c_iterator_get_end(field.prev_varsized_field, 'R'))
        _c('    return (%s *) ((char *) prev.data + XCB_TYPE_PAD(%s, prev.index) + %d);', field.c_field_type, field.first_field_after_varsized.type.c_type, field.prev_varsized_offset)
        _c('}')
    
def _c_accessors_list(self, field):
    '''
    Declares the accessor functions for a list field.
    Declares a direct-accessor function only if the list members are fixed size.
    Declares length and get-iterator functions always.
    '''
    list = field.type

    _h_setlevel(1)
    _c_setlevel(1)
    if list.member.fixed_size():
        _hc('')
        _hc('')
        _hc('/*****************************************************************************')
        _hc(' **')
        _hc(' ** %s * %s', field.c_field_type, field.c_accessor_name)
        _hc(' ** ')
        _hc(' ** @param const %s *R', self.c_type)
        _hc(' ** @returns %s *', field.c_field_type)
        _hc(' **')
        _hc(' *****************************************************************************/')
        _hc(' ')
        _hc('%s *', field.c_field_type)
        _h('%s (const %s *R  /**< */);', field.c_accessor_name, self.c_type)
        _c('%s (const %s *R  /**< */)', field.c_accessor_name, self.c_type)
        _c('{')

        if field.prev_varsized_field == None:
            _c('    return (%s *) (R + 1);', field.c_field_type)
        else:
            _c('    xcb_generic_iterator_t prev = %s;', _c_iterator_get_end(field.prev_varsized_field, 'R'))
            _c('    return (%s *) ((char *) prev.data + XCB_TYPE_PAD(%s, prev.index) + %d);', field.c_field_type, field.first_field_after_varsized.type.c_type, field.prev_varsized_offset)

        _c('}')

    _hc('')
    _hc('')
    _hc('/*****************************************************************************')
    _hc(' **')
    _hc(' ** int %s', field.c_length_name)
    _hc(' ** ')
    _hc(' ** @param const %s *R', self.c_type)
    _hc(' ** @returns int')
    _hc(' **')
    _hc(' *****************************************************************************/')
    _hc(' ')
    _hc('int')
    _h('%s (const %s *R  /**< */);', field.c_length_name, self.c_type)
    _c('%s (const %s *R  /**< */)', field.c_length_name, self.c_type)
    _c('{')
    _c('    return %s;', _c_accessor_get_expr(field.type.expr, 'R'))
    _c('}')

    if field.type.member.is_simple:
        _hc('')
        _hc('')
        _hc('/*****************************************************************************')
        _hc(' **')
        _hc(' ** xcb_generic_iterator_t %s', field.c_end_name)
        _hc(' ** ')
        _hc(' ** @param const %s *R', self.c_type)
        _hc(' ** @returns xcb_generic_iterator_t')
        _hc(' **')
        _hc(' *****************************************************************************/')
        _hc(' ')
        _hc('xcb_generic_iterator_t')
        _h('%s (const %s *R  /**< */);', field.c_end_name, self.c_type)
        _c('%s (const %s *R  /**< */)', field.c_end_name, self.c_type)
        _c('{')
        _c('    xcb_generic_iterator_t i;')

        if field.prev_varsized_field == None:
            _c('    i.data = ((%s *) (R + 1)) + (%s);', field.type.c_wiretype, _c_accessor_get_expr(field.type.expr, 'R'))
        else:
            _c('    xcb_generic_iterator_t child = %s;', _c_iterator_get_end(field.prev_varsized_field, 'R'))
            _c('    i.data = ((%s *) child.data) + (%s);', field.type.c_wiretype, _c_accessor_get_expr(field.type.expr, 'R'))

        _c('    i.rem = 0;')
        _c('    i.index = (char *) i.data - (char *) R;')
        _c('    return i;')
        _c('}')

    else:
        _hc('')
        _hc('')
        _hc('/*****************************************************************************')
        _hc(' **')
        _hc(' ** %s %s', field.c_iterator_type, field.c_iterator_name)
        _hc(' ** ')
        _hc(' ** @param const %s *R', self.c_type)
        _hc(' ** @returns %s', field.c_iterator_type)
        _hc(' **')
        _hc(' *****************************************************************************/')
        _hc(' ')
        _hc('%s', field.c_iterator_type)
        _h('%s (const %s *R  /**< */);', field.c_iterator_name, self.c_type)
        _c('%s (const %s *R  /**< */)', field.c_iterator_name, self.c_type)
        _c('{')
        _c('    %s i;', field.c_iterator_type)

        if field.prev_varsized_field == None:
            _c('    i.data = (%s *) (R + 1);', field.c_field_type)
        else:
            _c('    xcb_generic_iterator_t prev = %s;', _c_iterator_get_end(field.prev_varsized_field, 'R'))
            _c('    i.data = (%s *) ((char *) prev.data + XCB_TYPE_PAD(%s, prev.index));', field.c_field_type, field.c_field_type)

        _c('    i.rem = %s;', _c_accessor_get_expr(field.type.expr, 'R'))
        _c('    i.index = (char *) i.data - (char *) R;')
        _c('    return i;')
        _c('}')

def _c_accessors(self, name, base):
    '''
    Declares the accessor functions for the fields of a structure.
    '''
    for field in self.fields:
        if field.type.is_list and not field.type.fixed_size():
            _c_accessors_list(self, field)
        elif field.prev_varsized_field != None:
            _c_accessors_field(self, field)

def c_simple(self, name):
    '''
    Exported function that handles cardinal type declarations.
    These are types which are typedef'd to one of the CARDx's, char, float, etc.
    '''
    _c_type_setup(self, name, ())

    if (self.name != name):
        # Typedef
        _h_setlevel(0)
        my_name = _t(name)
        _h('')
        _h('typedef %s %s;', _t(self.name), my_name)

        # Iterator
        _c_iterator(self, name)

def _c_complex(self):
    '''
    Helper function for handling all structure types.
    Called for all structs, requests, replies, events, errors.
    '''
    _h_setlevel(0)
    _h('')
    _h('/**')
    _h(' * @brief %s', self.c_type)
    _h(' **/')
    _h('typedef %s %s {', self.c_container, self.c_type)

    struct_fields = []
    maxtypelen = 0

    varfield = None
    for field in self.fields:
        if not field.type.fixed_size():
            varfield = field.c_field_name
            continue
        if varfield != None and not field.type.is_pad and field.wire:
            errmsg = '%s: warning: variable field %s followed by fixed field %s\n' % (self.c_type, varfield, field.c_field_name)
            sys.stderr.write(errmsg)
            # sys.exit(1)
        if field.wire:
            struct_fields.append(field)
        
    for field in struct_fields:
        if len(field.c_field_type) > maxtypelen:
            maxtypelen = len(field.c_field_type)

    for field in struct_fields:
        spacing = ' ' * (maxtypelen - len(field.c_field_type))
        _h('    %s%s %s%s; /**<  */', field.c_field_type, spacing, field.c_field_name, field.c_subscript)

    _h('} %s;', self.c_type)

def c_struct(self, name):
    '''
    Exported function that handles structure declarations.
    '''
    _c_type_setup(self, name, ())
    _c_complex(self)
    _c_accessors(self, name, name)
    _c_iterator(self, name)

def c_union(self, name):
    '''
    Exported function that handles union declarations.
    '''
    _c_type_setup(self, name, ())
    _c_complex(self)
    _c_iterator(self, name)

def _c_request_helper(self, name, cookie_type, void, regular):
    '''
    Declares a request function.
    '''

    # Four stunningly confusing possibilities here:
    #
    #   Void            Non-void
    # ------------------------------
    # "req"            "req"
    # 0 flag           CHECKED flag   Normal Mode
    # void_cookie      req_cookie
    # ------------------------------
    # "req_checked"    "req_unchecked"
    # CHECKED flag     0 flag         Abnormal Mode
    # void_cookie      req_cookie
    # ------------------------------


    # Whether we are _checked or _unchecked
    checked = void and not regular
    unchecked = not void and not regular

    # What kind of cookie we return
    func_cookie = 'xcb_void_cookie_t' if void else self.c_cookie_type

    # What flag is passed to xcb_request
    func_flags = '0' if (void and regular) or (not void and not regular) else 'XCB_REQUEST_CHECKED'

    # Global extension id variable or NULL for xproto
    func_ext_global = '&' + _ns.c_ext_global_name if _ns.is_ext else '0'

    # What our function name is
    func_name = self.c_request_name
    if checked:
        func_name = self.c_checked_name
    if unchecked:
        func_name = self.c_unchecked_name

    param_fields = []
    wire_fields = []
    maxtypelen = len('xcb_connection_t')

    for field in self.fields:
        if field.visible:
            # The field should appear as a call parameter
            param_fields.append(field)
        if field.wire and not field.auto:
            # We need to set the field up in the structure
            wire_fields.append(field)
        
    for field in param_fields:
        if len(field.c_field_const_type) > maxtypelen:
            maxtypelen = len(field.c_field_const_type)

    _h_setlevel(1)
    _c_setlevel(1)
    _h('')
    _h('/**')
    _h(' * Delivers a request to the X server')
    _h(' * @param c The connection')
    _h(' * @return A cookie')
    _h(' *')
    _h(' * Delivers a request to the X server.')
    _h(' * ')
    if checked:
        _h(' * This form can be used only if the request will not cause')
        _h(' * a reply to be generated. Any returned error will be')
        _h(' * saved for handling by xcb_request_check().')
    if unchecked:
        _h(' * This form can be used only if the request will cause')
        _h(' * a reply to be generated. Any returned error will be')
        _h(' * placed in the event queue.')
    _h(' */')
    _c('')
    _hc('')
    _hc('/*****************************************************************************')
    _hc(' **')
    _hc(' ** %s %s', cookie_type, func_name)
    _hc(' ** ')

    spacing = ' ' * (maxtypelen - len('xcb_connection_t'))
    _hc(' ** @param xcb_connection_t%s *c', spacing)

    for field in param_fields:
        spacing = ' ' * (maxtypelen - len(field.c_field_const_type))
        _hc(' ** @param %s%s %s%s', field.c_field_const_type, spacing, field.c_pointer, field.c_field_name)

    _hc(' ** @returns %s', cookie_type)
    _hc(' **')
    _hc(' *****************************************************************************/')
    _hc(' ')
    _hc('%s', cookie_type)

    spacing = ' ' * (maxtypelen - len('xcb_connection_t'))
    comma = ',' if len(param_fields) else ');'
    _h('%s (xcb_connection_t%s *c  /**< */%s', func_name, spacing, comma)
    comma = ',' if len(param_fields) else ')'
    _c('%s (xcb_connection_t%s *c  /**< */%s', func_name, spacing, comma)

    func_spacing = ' ' * (len(func_name) + 2)
    count = len(param_fields)
    for field in param_fields:
        count = count - 1
        spacing = ' ' * (maxtypelen - len(field.c_field_const_type))
        comma = ',' if count else ');'
        _h('%s%s%s %s%s  /**< */%s', func_spacing, field.c_field_const_type, spacing, field.c_pointer, field.c_field_name, comma)
        comma = ',' if count else ')'
        _c('%s%s%s %s%s  /**< */%s', func_spacing, field.c_field_const_type, spacing, field.c_pointer, field.c_field_name, comma)

    count = 2
    for field in param_fields:
        if not field.type.fixed_size():
            count = count + 2

    _c('{')
    _c('    static const xcb_protocol_request_t xcb_req = {')
    _c('        /* count */ %d,', count)
    _c('        /* ext */ %s,', func_ext_global)
    _c('        /* opcode */ %s,', self.c_request_name.upper())
    _c('        /* isvoid */ %d', 1 if void else 0)
    _c('    };')
    _c('    ')
    _c('    struct iovec xcb_parts[%d];', count + 2)
    _c('    %s xcb_ret;', func_cookie)
    _c('    %s xcb_out;', self.c_type)
    _c('    ')

    for field in wire_fields:
        if field.type.fixed_size():
            if field.type.is_expr:
                _c('    xcb_out.%s = %s;', field.c_field_name, _c_accessor_get_expr(field.type.expr))

            elif field.type.is_pad:
                if field.type.nmemb == 1:
                    _c('    xcb_out.%s = 0;', field.c_field_name)
                else:
                    _c('    memset(xcb_out.%s, 0, %d);', field.c_field_name, field.type.nmemb)
            else:
                if field.type.nmemb == 1:
                    _c('    xcb_out.%s = %s;', field.c_field_name, field.c_field_name)
                else:
                    _c('    memcpy(xcb_out.%s, %s, %d);', field.c_field_name, field.c_field_name, field.type.nmemb)

    _c('    ')
    _c('    xcb_parts[2].iov_base = (char *) &xcb_out;')
    _c('    xcb_parts[2].iov_len = sizeof(xcb_out);')
    _c('    xcb_parts[3].iov_base = 0;')
    _c('    xcb_parts[3].iov_len = -xcb_parts[2].iov_len & 3;')

    count = 4
    for field in param_fields:
        if not field.type.fixed_size():
            _c('    xcb_parts[%d].iov_base = (char *) %s;', count, field.c_field_name)
            if field.type.is_list:
                _c('    xcb_parts[%d].iov_len = %s * sizeof(%s);', count, _c_accessor_get_expr(field.type.expr), field.type.member.c_wiretype)
            else:
                _c('    xcb_parts[%d].iov_len = %s * sizeof(%s);', count, 'Uh oh', field.type.c_wiretype)
            _c('    xcb_parts[%d].iov_base = 0;', count + 1)
            _c('    xcb_parts[%d].iov_len = -xcb_parts[%d].iov_len & 3;', count + 1, count)
            count = count + 2

    _c('    xcb_ret.sequence = xcb_send_request(c, %s, xcb_parts + 2, &xcb_req);', func_flags)
    _c('    return xcb_ret;')
    _c('}')

def _c_reply(self, name):
    '''
    Declares the function that returns the reply structure.
    '''
    spacing1 = ' ' * (len(self.c_cookie_type) - len('xcb_connection_t'))
    spacing2 = ' ' * (len(self.c_cookie_type) - len('xcb_generic_error_t'))
    spacing3 = ' ' * (len(self.c_reply_name) + 2)

    _h('')
    _h('/**')
    _h(' * Return the reply')
    _h(' * @param c      The connection')
    _h(' * @param cookie The cookie')
    _h(' * @param e      The xcb_generic_error_t supplied')
    _h(' *')
    _h(' * Returns the reply of the request asked by')
    _h(' * ')
    _h(' * The parameter @p e supplied to this function must be NULL if')
    _h(' * %s(). is used.', self.c_unchecked_name)
    _h(' * Otherwise, it stores the error if any.')
    _h(' *')
    _h(' * The returned value must be freed by the caller using free().')
    _h(' */')
    _c('')
    _hc('')
    _hc('/*****************************************************************************')
    _hc(' **')
    _hc(' ** %s * %s', self.c_reply_type, self.c_reply_name)
    _hc(' ** ')
    _hc(' ** @param xcb_connection_t%s  *c', spacing1)
    _hc(' ** @param %s   cookie', self.c_cookie_type)
    _hc(' ** @param xcb_generic_error_t%s **e', spacing2)
    _hc(' ** @returns %s *', self.c_reply_type)
    _hc(' **')
    _hc(' *****************************************************************************/')
    _hc(' ')
    _hc('%s *', self.c_reply_type)
    _hc('%s (xcb_connection_t%s  *c  /**< */,', self.c_reply_name, spacing1)
    _hc('%s%s   cookie  /**< */,', spacing3, self.c_cookie_type)
    _h('%sxcb_generic_error_t%s **e  /**< */);', spacing3, spacing2)
    _c('%sxcb_generic_error_t%s **e  /**< */)', spacing3, spacing2)
    _c('{')
    _c('    return (%s *) xcb_wait_for_reply(c, cookie.sequence, e);', self.c_reply_type)
    _c('}')

def _c_opcode(name, opcode):
    '''
    Declares the opcode define for requests, events, and errors.
    '''
    _h_setlevel(0)
    _h('')
    _h('/** Opcode for %s. */', _n(name))
    _h('#define %s %s', _n(name).upper(), opcode)
    
def _c_cookie(self, name):
    '''
    Declares the cookie type for a non-void request.
    '''
    _h_setlevel(0)
    _h('')
    _h('/**')
    _h(' * @brief %s', self.c_cookie_type)
    _h(' **/')
    _h('typedef struct %s {', self.c_cookie_type)
    _h('    unsigned int sequence; /**<  */')
    _h('} %s;', self.c_cookie_type)

def c_request(self, name):
    '''
    Exported function that handles request declarations.
    '''
    _c_type_setup(self, name, ('request',))

    if self.reply:
        # Cookie type declaration
        _c_cookie(self, name)

    # Opcode define
    _c_opcode(name, self.opcode)

    # Request structure declaration
    _c_complex(self)

    if self.reply:
        _c_type_setup(self.reply, name, ('reply',))
        # Reply structure definition
        _c_complex(self.reply)
        # Request prototypes
        _c_request_helper(self, name, self.c_cookie_type, False, True)
        _c_request_helper(self, name, self.c_cookie_type, False, False)
        # Reply accessors
        _c_accessors(self.reply, name + ('reply',), name)
        _c_reply(self, name)
    else:
        # Request prototypes
        _c_request_helper(self, name, 'xcb_void_cookie_t', True, False)
        _c_request_helper(self, name, 'xcb_void_cookie_t', True, True)

def c_event(self, name):
    '''
    Exported function that handles event declarations.
    '''
    _c_type_setup(self, name, ('event',))

    # Opcode define
    _c_opcode(name, self.opcodes[name])

    if self.name == name:
        # Structure definition
        _c_complex(self)
    else:
        # Typedef
        _h('')
        _h('typedef %s %s;', _t(self.name + ('event',)), _t(name + ('event',)))

def c_error(self, name):
    '''
    Exported function that handles error declarations.
    '''
    _c_type_setup(self, name, ('error',))

    # Opcode define
    _c_opcode(name, self.opcodes[name])

    if self.name == name:
        # Structure definition
        _c_complex(self)
    else:
        # Typedef
        _h('')
        _h('typedef %s %s;', _t(self.name + ('error',)), _t(name + ('error',)))


# Main routine starts here

# Must create an "output" dictionary before any xcbgen imports.
output = {'open'    : c_open,
          'close'   : c_close,
          'simple'  : c_simple,
          'enum'    : c_enum,
          'struct'  : c_struct,
          'union'   : c_union,
          'request' : c_request,
          'event'   : c_event,
          'error'   : c_error
          }

# Boilerplate below this point

# Check for the argument that specifies path to the xcbgen python package.
try:
    opts, args = getopt.getopt(sys.argv[1:], 'p:')
except getopt.GetoptError, err:
    print str(err)
    print 'Usage: c_client.py [-p path] file.xml'
    sys.exit(1)

for (opt, arg) in opts:
    if opt == '-p':
        sys.path.append(arg)

# Import the module class
try:
    from xcbgen.state import Module
except ImportError:
    print ''
    print 'Failed to load the xcbgen Python package!'
    print 'Make sure that xcb/proto installed it on your Python path.'
    print 'If not, you will need to create a .pth file or define $PYTHONPATH'
    print 'to extend the path.'
    print 'Refer to the README file in xcb/proto for more info.'
    print ''
    raise

# Parse the xml header
module = Module(args[0], output)

# Build type-registry and resolve type dependencies
module.register()
module.resolve()

# Output the code
module.generate()
