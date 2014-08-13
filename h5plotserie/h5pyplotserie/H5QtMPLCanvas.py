from PyQt4 import QtGui, QtCore

from numpy import arange, sin, pi
from matplotlib.backends.backend_qt4agg import FigureCanvasQTAgg as FigureCanvas
from matplotlib.backends.backend_qt4agg import NavigationToolbar2QTAgg as NavigationToolbar
from matplotlib.figure import Figure

class Canvas(FigureCanvas):
    """
    Ultimately, this is a QWidget (as well as a FigureCanvasAgg, etc.).+
    copied from http://matplotlib.org/examples/user_interfaces/embedding_in_qt4.html on 23.06.2013
    """
    def __init__(self, parent=None, width=1, height=1, dpi=100):
        self.fig = Figure(figsize=(width, height), dpi=dpi)
        super(Canvas, self).__init__(self.fig)
        
        # Set up axes
        self.axes = self.fig.add_subplot(111)
        self.axes.hold(True)
        self.axes.axvline(linewidth=1, color='black')
        
        self.setParent(parent)
        
        # Event handling (http://matplotlib.org/users/event_handling.html)
#         self.mpl_connect('button_press_event', self.onclick)
#         self.mpl_connect('scroll_event', self.onMouseWheel)

        FigureCanvas.setSizePolicy(self, QtGui.QSizePolicy.Expanding, QtGui.QSizePolicy.Expanding)
        FigureCanvas.updateGeometry(self)
        
    def addPlot(self, x, y):
        def avoidNonZeroZeros(vec):
            '''
            In matplotlib there is a bug (at least in V1.3.1. that vectors that hold basically just zeros except that they are not zero (e.g. 1e-302) that the internal computations are not done correctly. Therefore these vectors are transformed here 
            '''
            # Check if the maximal as well as the minmal value are basicall< zero
            if (1 / max(vec)) == (1 / min(vec)) == float('Inf'):
                # Convert all values to zero
                for i in xrange(len(vec)):
                    vec[i] = 0
            return vec
                    
        self.axes.plot(avoidNonZeroZeros(x), avoidNonZeroZeros(y))
        self.draw()
        
    def clear(self):
        self.axes.clear()
        
    def toggle_grid(self):
        self.axes.grid(color='black', linestyle=':', linewidth=0.5)
        self.draw()
        
    def onclick(self, event):
        self.axes.xaxis.pan(-100)
        print 'button=%d, x=%d, y=%d, xdata=%f, ydata=%f' % (
            event.button, event.x, event.y, event.xdata, event.ydata)
    
    
class PlotWindow(QtGui.QWidget):
    def __init__(self):
        super(PlotWindow, self).__init__()
        
        self.initUI()
        
    def initUI(self):
        mainLayout = QtGui.QGridLayout()
        self.setLayout(mainLayout)
        
        # List of added Plots
        panel = QtGui.QWidget()
        mainLayout.addWidget(panel, 1, 0)
        
        panelLayout = QtGui.QGridLayout()
        panel.setLayout(panelLayout)
        
        self.plotsList = QtGui.QComboBox()
        panelLayout.addWidget(self.plotsList, 0, 0)
        self.plotsList.activated.connect(self.changeCurrentPlot)
        
        self.xStringEdit = QtGui.QLineEdit()
        panelLayout.addWidget(self.xStringEdit, 1, 0)
        self.yStringEdit = QtGui.QLineEdit()
        panelLayout.addWidget(self.yStringEdit, 2, 0)
         
        
        # Add canvas
        self.Canvas = Canvas()
        mainLayout.addWidget(self.Canvas, 3, 0) 
        
        self.toolbar = NavigationToolbar(self.Canvas, self)
        mainLayout.addWidget(self.toolbar, 2, 0) 

        
        # Add menu-bar
        menu = QtGui.QMenuBar()
        
        menuPlot = menu.addMenu("&Plot")
        
        # Add clear
        clearAction = QtGui.QAction("&Clear", self)
        clearAction.setShortcut(QtGui.QKeySequence("Ctrl+C"))
        clearAction.triggered.connect(self.clear)
        menuPlot.addAction(clearAction)
        
        # Add grid
        gridAction = QtGui.QAction("&Grid", self)
        gridAction.setShortcut(QtGui.QKeySequence("Ctrl+G"))
        gridAction.triggered.connect(self.Canvas.toggle_grid)
        menuPlot.addAction(gridAction)  
        
        exportAction = QtGui.QAction("&Export", self)
        exportAction.setShortcut(QtGui.QKeySequence("Ctrl+E"))
        exportAction.triggered.connect(self.export)
        menuPlot.addAction(exportAction)    
        
        mainLayout.addWidget(menu, 0, 0)
        
     
        
    def addPlot(self, x, y, name, xString, yString):
        userData = [xString, yString]
        self.plotsList.addItem(name, userData=userData)
        self.changeCurrentPlot()
        self.Canvas.addPlot(x, y)
        
    
    def clear(self):
        self.plotsList.clear()
        self.Canvas.clear()
        
    def export(self):
        '''
        Export the data (for example to tikz?)
        '''
        for line in self.Canvas.axes.get_lines():
            xData = line.get_xdata()
            yData = line.get_ydata()
        
    def changeCurrentPlot(self):
        'TODO' 
        test = self.plotsList.itemData(self.plotsList.currentIndex())
        self.xStringEdit.setText(test.toList()[0].toString())
        self.yStringEdit.setText(test.toList()[1].toString())
        
