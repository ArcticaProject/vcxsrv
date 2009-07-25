'''
This module contains helper classes for structure fields and length expressions.
'''
class Field(object):
    '''
    Represents a field of a structure.

    type is the datatype object for the field.
    field_type is the name of the type (string tuple)
    field_name is the name of the structure field.
    visible is true iff the field should be in the request API.
    wire is true iff the field should be in the request structure.
    auto is true iff the field is on the wire but not in the request API (e.g. opcode)
    '''
    def __init__(self, type, field_type, field_name, visible, wire, auto):
        self.type = type
        self.field_type = field_type
        self.field_name = field_name
        self.visible = visible
        self.wire = wire
        self.auto = auto


class Expression(object):
    '''
    Represents a mathematical expression for a list length or exprfield.

    Public fields:
    op is the operation (text +,*,/,<<) or None.
    lhs and rhs are the sub-Expressions if op is set.
    lenfield_name is the name of the length field, or None for request lists.
    lenfield is the Field object for the length field, or None.
    bitfield is True if the length field is a bitmask instead of a number.
    nmemb is the fixed size (value)of the expression, or None
    '''
    def __init__(self, elt, parent):
        self.parent = parent

        self.nmemb = None

        self.lenfield_name = None
        self.lenfield_type = None
        self.lenfield = None
        self.lenwire = False
        self.bitfield = False

        self.op = None
        self.lhs = None
        self.rhs = None

        if elt.tag == 'list':
            # List going into a request, which has no length field (inferred by server)
            self.lenfield_name = elt.get('name') + '_len'
            self.lenfield_type = 'CARD32'

        elif elt.tag == 'fieldref':
            # Standard list with a fieldref
            self.lenfield_name = elt.text

        elif elt.tag == 'valueparam':
            # Value-mask.  The length bitmask is described by attributes.
            self.lenfield_name = elt.get('value-mask-name')
            self.lenfield_type = elt.get('value-mask-type')
            self.lenwire = True
            self.bitfield = True

        elif elt.tag == 'op':
            # Op field.  Need to recurse.
            self.op = elt.get('op')
            self.lhs = Expression(list(elt)[0], parent)
            self.rhs = Expression(list(elt)[1], parent)

            # Hopefully we don't have two separate length fields...
            self.lenfield_name = self.lhs.lenfield_name
            if self.lenfield_name == None:
                self.lenfield_name = self.rhs.lenfield_name

        elif elt.tag == 'value':
            # Constant expression
            self.nmemb = int(elt.text)

        else:
            # Notreached
            raise Exception('XXX')


    def fixed_size(self):
        return self.nmemb != None
