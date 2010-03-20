#!/usr/bin/python

from xml.sax.saxutils import XMLFilterBase, XMLGenerator
from xml.sax import make_parser
import sys, os

path = [os.path.curdir, 'extensions']
def find_file_on_path(name):
	for d in path:
		test = os.path.join(d, name)
		if os.path.isfile(test):
			return test
	raise OSError(errno.ENOENT, os.strerror(errno.ENOENT), name)

seen = {}

class ProcessImports(XMLFilterBase):
	def setContentHandler(self, handler):
		self.handler = handler
		XMLFilterBase.setContentHandler(self, handler)

	def ensure(self, name):
		if not seen.has_key(name):
			child = ProcessImports(make_parser())
			child.setContentHandler(self.handler)
			child.parse(find_file_on_path(name + '.xml'))

	def startDocument(self):
		pass
	def endDocument(self):
		pass

	inimport = None

	def startElement(self, name, attrs):
		assert self.inimport is None
		if name == 'import':
			self.inimport = ""
			return
		XMLFilterBase.startElement(self, name, attrs)
		if name == 'xcb':
			seen[attrs['header']] = True
			self.ensure('xproto')

	def characters(self, content):
		if self.inimport is not None:
			self.inimport += content
		else:
			XMLFilterBase.characters(self, content)

	def endElement(self, name):
		if name == 'import':
			self.ensure(self.inimport)
			self.inimport = None
			return
		XMLFilterBase.endElement(self, name)

out = XMLGenerator()
importer = ProcessImports(make_parser())
importer.setContentHandler(out)
out.startDocument()
if len(sys.argv) > 1:
	importer.parse(sys.argv[1])
else:
	importer.parse(sys.stdin)
out.endDocument()
