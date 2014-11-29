#! /usr/bin/env python

import xml.dom.minidom as xdom
import shutil
import sys
import os
import re

whiteList = "mediatek/build/tools/preprocess/whiteList.xml"

if not os.path.isfile(whiteList):
    print >> sys.stdout,"can not find [whiteList] '%s'!" % whiteList
    sys.exit(0)

fileType = "-name *.cjava -o -name *.cxml -o -name *.ctxt"

class XmlDom(object):
    def __init__(self,xml):
        self.xmlDom = xdom.parse(xml)

    def getRoot(self):
        return self.xmlDom.documentElement

    def getFileList(self):
        return self.getElementSafe("FileList")

    def getUnrecusiveDirList(self):
        return self.getElementSafe("UnrecursiveDirList")

    def getRecusiveDirList(self):
        return self.getElementSafe("RecursiveDirList")

    def getElementSafe(self, elemtTag):
        root = self.getRoot()
        if root is None:
            return root
        note = root.getElementsByTagName(elemtTag)
        if len(note) == 0:
            return note
        notetList = note[0].getElementsByTagName("List")
        if len(notetList) == 0:
            return notetList
        elementList = map(str,[item.firstChild.nodeValue for item in notetList if item.firstChild is not None])
        return elementList

#end XmlDom

# create custom release policy DOM
dom = XmlDom(whiteList)

fileList = dom.getFileList()
unrecusiveDirList = dom.getUnrecusiveDirList()
recusiveDirList= dom.getRecusiveDirList()

class AllFiles(object):
    def __init__(self):
        """ get files initialization """
        self.fileList = dom.getFileList()
        self.unrecusiveDirList = dom.getUnrecusiveDirList()
        self.recusiveDirList= dom.getRecusiveDirList()

    def getFiles(self):
        allFiles = []
        allFiles.extend(self.getFilesInfileList())
        allFiles.extend(self.getFilesInUnrecusiveDir())
        allFiles.extend(self.getFilesInRecusiveDir())
        return allFiles

    def getFilesInfileList(self):
        fileInFileList = []
        for eachFile in self.fileList:
            if os.path.isfile(eachFile):
                fileInFileList.append(eachFile)
        return fileInFileList

    def getFilesInUnrecusiveDir(self):
        fileInUnrecusiveDir = []
        for eachDir in unrecusiveDirList:
            fileInUnrecusiveDir.extend(map(lambda x:x.rstrip(),list(os.popen("find %s -maxdepth 1 %s" % (eachDir, fileType)))))
        return fileInUnrecusiveDir

    def getFilesInRecusiveDir(self):
        fileInRecusiveDir = []
        for eachDir in recusiveDirList:
            fileInRecusiveDir.extend(map(lambda x:x.rstrip(),list(os.popen("find %s %s" % (eachDir, fileType)))))
        return fileInRecusiveDir

allFils = AllFiles()
"""
print >> sys.stdout,"[fileList] '%s'" % " ".join(map(str,fileList))
print >> sys.stdout,"[unrecusiveDirList] '%s'" % " ".join(map(str,unrecusiveDirList))
print >> sys.stdout,"[recusiveDirList] '%s'" % " ".join(map(str,recusiveDirList))
"""
print >> sys.stdout,"%s" % " ".join(map(str,allFils.getFiles()))



