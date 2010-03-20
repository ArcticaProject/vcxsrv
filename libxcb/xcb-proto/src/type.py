#!/usr/bin/python

from xml.sax.saxutils import XMLFilterBase, XMLGenerator
from xml.sax.xmlreader import AttributesImpl
from xml.sax import make_parser
import sys

def AttributesUnion(base, **values):
	baseitems = dict(base)
	baseitems.update(values)
	return AttributesImpl(baseitems)

class AnnotateType(XMLFilterBase):
	scopes = []
	map = dict([(name, [name]) for name in [
			'BOOL', 'BYTE',
			'CARD8', 'CARD16', 'CARD32',
			'INT8', 'INT16', 'INT32',
			'char', 'void',
			'float', 'double',
			'XID',
		]])
	def startScope(self, name):
		self.scopes.insert(0, name)
	def declareType(self, name):
		assert ':' not in name
		qname = self.scopes[0] + ':' + name
		self.map.setdefault(name, []).insert(0, qname)
	def getQualifiedType(self, name):
		if ':' in name:
			return name
		names = self.map.get(name, [])
		return names[0]
	def endScope(self):
		self.scopes.pop(0)

	def startElement(self, name, attrs):
		attnames = []
		if name == 'xcb':
			self.startScope(attrs['header'])
		elif name in ['struct', 'union', 'xidtype', 'enum', 'event', 'eventcopy', 'error', 'errorcopy']:
			self.declareType(attrs['name'])
			attnames = ['name']
			if name.endswith('copy'):
				attnames.append('ref')
		elif name == 'typedef':
			self.declareType(attrs['newname'])
			attnames = ['oldname', 'newname']
		elif name == 'valueparam':
			attnames = ['value-mask-type']
		elif attrs.has_key('type'):
			attnames = ['type']
		newattrs = {}
		for attname in attnames:
			newattrs[attname] = self.getQualifiedType(attrs[attname])
		if newattrs:
			attrs = AttributesUnion(attrs, **newattrs)
		XMLFilterBase.startElement(self, name, attrs)

	def endElement(self, name):
		XMLFilterBase.endElement(self, name)
		if name == 'xcb':
			self.endScope()

annotator = AnnotateType(make_parser())
annotator.setContentHandler(XMLGenerator())
if len(sys.argv) > 1:
	annotator.parse(sys.argv[1])
else:
	annotator.parse(sys.stdin)

for name,names in annotator.map.iteritems():
	if len(names) != 1:
		print "<!-- warning:", name, "has the following definitions:", names, "-->"
