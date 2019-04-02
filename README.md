# QtCells
Qt project to draw arrow links between cell in QTableWidget.
**QPainter** is implemented to the table widget derived from **QTableWidget** as standard _paintEvent_ is not defined to draw polygons and lines.

Links may be drawn just between cells with text (e.g. nodes).
Drag&Drop is enabled. When moving the node, the links associated with this node are also rebuilt.

![Nodes & links](https://github.com/IntergraDev/QtTablePainter/blob/master/manual/screen_001.PNG)

# Build
To build the application QtCreator should installed. First use
```qmake QtCells.pro``` in root.

Then, after the .pro file is created use ```mingw32-make``` to compile.
Finally change the dir with ```cd release``` and use ```windeployqt.exe ./``` to collect required DLLs.

Launch **QtCells.exe**
