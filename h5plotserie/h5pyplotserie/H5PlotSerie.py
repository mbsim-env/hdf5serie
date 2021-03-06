import sys
import os
from PyQt4 import QtGui, QtCore, Qt
from PyQt4.Qt import QVariant

import HDF5Serie
import H5QtMPLCanvas
from xdg.Config import setWindowManager

class MyQListWidget(QtGui.QListWidget):
    '''
    QListWidget-Class that enables to track mouse press event 
    Copied from http://stackoverflow.com/questions/9312287/catch-which-mousebutton-is-pressed-on-item on 31.01.2014
    '''
    def __init__(self, parent=None):
        super(MyQListWidget, self).__init__(parent)

    def mousePressEvent(self, event):
        self._mouse_button = event.button()
        super(MyQListWidget, self).mousePressEvent(event)

class H5PlotSerie(QtGui.QMainWindow):
    def __init__(self, args=[]):
        super(H5PlotSerie, self).__init__()
        
        self.initUI()
        self.h5Files = []
        self.currentDir = os.curdir
        self.setAcceptDrops(True) 
        self.showMaximized()
        
        self.xInd = 0
        self.yInd = 1

        self.setWindowTitle('H5PyPlotSerie')
        for arg in args[1:]:
            self.addFile(str(arg))
        
    def initUI(self):
        centralW = QtGui.QWidget()
        self.setCentralWidget(centralW)
        self.mainLayout = QtGui.QGridLayout()
        centralW.setLayout(self.mainLayout)

        self.searchEdit = QtGui.QLineEdit()
        self.searchEdit.setMaximumWidth(500)
        self.mainLayout.addWidget(self.searchEdit, 0, 0)
        self.connect(self.searchEdit, QtCore.SIGNAL('returnPressed()'), self.updateTree)
        
        # Add tree widget
        self.treeGroups = QtGui.QTreeWidget()
        self.treeGroups.setMaximumWidth(500)
        self.mainLayout.addWidget(self.treeGroups, 1, 0)                
        self.connect(self.treeGroups, QtCore.SIGNAL('itemSelectionChanged()'), self.loadData)
        self.treeGroups.setContextMenuPolicy(QtCore.Qt.ActionsContextMenu)

        attributes = QtGui.QAction("Attributes", self)
        self.treeGroups.addAction(attributes)
        self.connect(attributes, QtCore.SIGNAL("triggered()"), self.showAttributes)
        
        unloadFile = QtGui.QAction("Unload", self)
        self.treeGroups.addAction(unloadFile)
        self.connect(unloadFile, QtCore.SIGNAL("triggered()"), self.unloadCurrentFile)
        
        #
        self.listData = MyQListWidget()
        self.listData.setMaximumWidth(500)
        self.mainLayout.addWidget(self.listData, 2, 0)
        self.listData.itemClicked.connect(self.listClick)
        
        self.plots = QtGui.QTabWidget()        
        self.mainLayout.addWidget(self.plots, 0, 1, 0, 1)
        
        self.plots.setMovable(True)
        self.plots.setTabsClosable(True)
        self.plots.tabCloseRequested.connect(self.closePlot)
        
        
        # Add Menu bar
        menu = QtGui.QMenuBar()
        file = menu.addMenu("File...")
        self.setMenuBar(menu)
        
        self.loadAction = QtGui.QAction("Load File", None)
        self.loadAction.setShortcut(QtGui.QKeySequence("Ctrl+L"))
        file.addAction(self.loadAction)
        self.connect(self.loadAction, QtCore.SIGNAL("triggered()"), self.loadFile)
        
        self.quitAction = QtGui.QAction("Quit", None)
        self.quitAction.setShortcut(QtGui.QKeySequence("Ctrl+Q"))
        file.addAction(self.quitAction)
        self.connect(self.quitAction, QtCore.SIGNAL("triggered()"), self.close)
    
    def getH5File(self, filename):
        for item in self.h5Files:
            if filename == item.filename:
                return item
        else:
            return None
        
    def searchItems(self, h5File):
        expr = str(self.searchEdit.text())        
        return h5File.findAllGroups(expr)
        
    def dragEnterEvent(self, event):
        if event.mimeData().hasUrls():
            event.acceptProposedAction()
        else:
            event.ignore() 

    def dropEvent(self, event):
        for url in event.mimeData().urls():
            if url.isLocalFile():
                path = url.toLocalFile()
                path = str(path.toLocal8Bit().data())
            else:
                # Copied from http://askubuntu.com/questions/213638/how-to-find-a-samba-shares-gvfs-path on 2015-02-18
                from gi.repository import Gio
                gvfs = Gio.Vfs.get_default()
                path = str(url.toString())
                path = gvfs.get_file_for_uri(path).get_path()
            self.addFile(path)
        
    def loadFile(self):
        filename = QtGui.QFileDialog.getOpenFileNameAndFilter(self, 'Load File', filter='*.mbsh5', directory=self.currentDir)
        
        if filename is not None:
            filename = os.path.abspath(str(filename[0]))
            self.addFile(filename)
            
    def addFile(self, filename):
        '''
        Add File to list
        '''
        try:
            if os.path.exists(filename):
                if filename.endswith('mbsh5') or filename.endswith('ombvh5'):
                    if self.getH5File(filename) is None:
                        self.h5Files.append(HDF5Serie.HDF5Serie(filename, 'r'))
                        self.currentDir = os.path.dirname(filename)
        except:
            pass
        
        self.updateTree()
        
    def unloadFile(self, filename):
        '''
        Unloads the given filename from the list
        '''
        self.h5Files.remove(self.getH5File(filename))
        self.updateTree()
        
    def showAttributes(self):
        '''
        Show all attributes of the Data
        '''
        try:
            attributes = self.treeGroups.currentItem().data(2, QtCore.Qt.UserRole).toPyObject()
            text = ''
            for attr in attributes:
                text = text + attr + '\n  ' + str(attributes[attr]) + '\n\n'
            QtGui.QMessageBox.information(self, 'Attributes', text)
        except:
            print 'Could not show attributes...'
            pass
            
        
    def unloadCurrentFile(self):
        '''
        Unloads (removes) the current selected file from the context
        '''
        filename = self.treeGroups.currentItem().data(0, QtCore.Qt.UserRole).toString()
        self.unloadFile(str(filename))

    def updateTree(self):
        '''
        update the tree with respect to the search
        '''
        self.listData.clear()
        self.treeGroups.clear()
        for item in self.h5Files:
            self.addTree(item, self.searchItems(item))
                
    def addTree(self, h5File, subgroups):
        root = QtGui.QTreeWidgetItem()
        filename = h5File.filename
        text = filename.split('/')[-1].split('\\')[-1]
        root.setText(0, text)
        root.setToolTip(0, filename)
        data = QVariant(filename)
        root.setData(0, QtCore.Qt.UserRole, data)
        self.treeGroups.addTopLevelItem(root)  
        if len(subgroups) <= 3:
            root.setExpanded(True)      
        self.addSelectedSubitems(root, h5File, subgroups)
        
        
    def addSelectedSubitems(self, parentTree, parentGroup, subgroups):
        for groupname in sorted(subgroups):
            # Add only the tree of the selected subitem
            subItem = self.createSubitem(parentTree, parentGroup, groupname)
            parentTree.addChild(subItem)
            try:
                if len(subgroups[groupname]) <= 3:
                    subItem.setExpanded(True)
            except:
                pass
                        
            self.addSelectedSubitems(subItem, parentGroup[groupname], subgroups[groupname])
        
        # If there are no subgroups found any more then add all the rest to the list
        if len(subgroups) == 0:    
            self.addAllSubitems(parentTree, parentGroup)
    
    def addAllSubitems(self, parentTree, parentGroup):
        '''
        Add all Subitems of the given group to the parent tree
        '''
        try:
            for group in parentGroup:
                subItem = self.createSubitem(parentTree, parentGroup, group)
                parentTree.addChild(subItem)
                self.addAllSubitems(subItem, parentGroup[group])
        except:
            pass
        
    def createSubitem(self, parentItem, parentGroup, groupname):
        if groupname != 'data':
            attributes = parentGroup[groupname].attrs
            subItem = QtGui.QTreeWidgetItem()
            subItem.setText(0, groupname)        
            filenameData = QVariant(str(parentItem.data(0, QtCore.Qt.UserRole).toString()))
            parentPath = str(parentItem.data(1, QtCore.Qt.UserRole).toString()).replace('/data','') # The possible data has to be stripped!
            if 'data' in parentGroup[groupname]:
                subgroupData = QVariant(parentPath + '/' + groupname + '/data')
            else:
                subgroupData = QVariant(parentPath + '/' + groupname)
            subItem.setData(0, QtCore.Qt.UserRole, filenameData)
            subItem.setData(1, QtCore.Qt.UserRole, subgroupData)
            subItem.setExpanded(True)
            
            try:
                subItem.setData(2, QtCore.Qt.UserRole, attributes)
                if 'Description' in attributes:
                    subItem.setToolTip(0, attributes['Description'])
            except:
                pass
                        
            
            return subItem
        
                
    def getItemData(self, item = None):
        if item is None:
            selItems = self.treeGroups.selectedItems()
            if len(selItems) == 0:
                return
        item = selItems[0]
        filename = item.data(0, QtCore.Qt.UserRole).toString()
        internalTree = item.data(1, QtCore.Qt.UserRole).toString()
        return str(filename), str(internalTree)
                
    def loadData(self): 
        filename, internalTree = self.getItemData()
        if internalTree.endswith('/data'):
            labels = self.getH5File(filename).getColumnLabels(internalTree)
            
            self.listData.clear()
            self.listData.addItems(labels)
            self.changexIndex(0)
            
    def changexIndex(self, ind):
        self.listData.item(self.xInd).setTextColor(QtCore.Qt.black)
        self.listData.item(ind).setTextColor(QtCore.Qt.red)
        self.xInd = ind
            
    def listClick(self):
        # Add Plot to dataset
        if self.listData._mouse_button == QtCore.Qt.LeftButton: 
            self.yInd = self.listData.currentIndex().row()
            self.overwritePlot()
            
        elif self.listData._mouse_button == QtCore.Qt.RightButton:
            self.changexIndex(self.listData.currentIndex().row())
        
        elif self.listData._mouse_button == QtCore.Qt.MidButton:
            self.yInd = self.listData.currentIndex().row()
            self.addPlot()
            
    def getCurrentTab(self):
        if self.plots.currentWidget() is None:
            return self.addCanvas()
        else:
            return self.plots.currentWidget()
        
    def addCanvas(self):
        # Add canvas
        newPlotWindow = H5QtMPLCanvas.PlotWindow()
        name = 'Plot_' + str(len(self.plots))
        self.plots.addTab(newPlotWindow, name) 
        return newPlotWindow
            
    def createPlotData(self):
        filename, internalTree = self.getItemData()
        data = self.getH5File(filename).getData(internalTree)
        x = data[:, self.xInd]
        y = data[:, self.yInd]
        
        labels = self.getH5File(filename).getColumnLabels(internalTree)
        headLine = str(labels[self.yInd]) + ' vs. ' + str(labels[self.xInd])
        xString = 'x = h5read(\'' + os.path.abspath(filename) + '\',\'' + internalTree + '\',' + '[' + str(self.xInd + 1) + ',1], [1,' + str(len(x)) + ']);'
        yString = 'y = h5read(\'' + os.path.abspath(filename) + '\',\'' + internalTree + '\',' + '[' + str(self.yInd + 1) + ',1], [1,' + str(len(y)) + ']);'
        
        return x, y, headLine, xString, yString
           
    def overwritePlot(self):
        self.getCurrentTab().clear()
        self.addPlot()
            
    def addPlot(self):
        x, y, headLine, xString, yString = self.createPlotData()
        self.getCurrentTab().addPlot(x, y, headLine, xString, yString)
            
    def closePlot(self, i):
        self.plots.removeTab(i)
            
