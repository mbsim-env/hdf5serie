from PyQt4 import QtGui
import sys
import H5PlotSerie

app = QtGui.QApplication(sys.argv)
h5plot = H5PlotSerie.H5PlotSerie()
h5plot.show()
sys.exit(app.exec_())