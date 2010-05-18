#!/usr/bin/python

from xml.sax.saxutils import XMLFilterBase, XMLGenerator
from xml.sax.xmlreader import AttributesImpl
from xml.sax import make_parser
import sys

def AttributesUnion(base, **values):
	baseitems = dict(base)
	baseitems.update(values)
	return AttributesImpl(baseitems)

class AnnotateSize(XMLFilterBase):
	types = {
		'BYTE': 1, 'BOOL': 1,
		'CARD8': 1, 'CARD16': 2, 'CARD32': 4,
		'INT8': 1, 'INT16': 2, 'INT32': 4,
		'char': 1, 'void': 1,
		'float': 4, 'double': 8,
		'XID': 4,
	}
	header = []
	def setTypeSize(self, name, size):
		assert not self.types.has_key(name), "size of " + name + " declared as both " + str(size) + " and " + str(self.types[name])
		self.types[name] = size

	struct = None
	union = None
	def startElement(self, name, attrs):
		if name == 'xcb':
			self.header.insert(0, attrs['header'])
		elif name == 'field':
			size = self.types.get(attrs['type'], 0)
			if self.struct is not None:
				self.totalsize += size
			elif self.union is not None:
				self.totalsize = max(self.totalsize, size)
			attrs = AttributesUnion(attrs, bytes=str(size))
		elif name == 'pad':
			assert self.union is None
			if self.struct is not None:
				self.totalsize += int(attrs['bytes'])
		elif name == 'xidtype':
			self.setTypeSize(attrs['name'], 4)
		elif name == 'typedef':
			self.setTypeSize(attrs['newname'], self.types[attrs['oldname']])
		elif name == 'struct' or name == 'union':
			assert self.struct is None and self.union is None
			setattr(self, name, attrs['name'])
			self.totalsize = 0

		if len(self.header) == 1 or name == 'xcb':
			XMLFilterBase.startElement(self, name, attrs)

	def characters(self, content):
		if len(self.header) == 1:
			XMLFilterBase.characters(self, content)

	def endElement(self, name):
		if len(self.header) == 1 or name == 'xcb':
			XMLFilterBase.endElement(self, name)

		if name == 'xcb':
			self.header.pop(0)
		elif name == 'struct' or name == 'union':
			assert getattr(self, name) is not None
			self.setTypeSize(getattr(self, name), self.totalsize)
			setattr(self, name, None)
			del self.totalsize

annotator = AnnotateSize(make_parser())
annotator.setContentHandler(XMLGenerator())
if len(sys.argv) > 1:
	annotator.parse(sys.argv[1])
else:
	annotator.parse(sys.stdin)
