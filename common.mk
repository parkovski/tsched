cppobjs := app.o frame.o weekview.o dayview.o trussthumbview.o bedviewdlg.o \
bedview.o truss.o simpletruss.o floortruss.o trereader.o daypickerdlg.o \
treatasdlg.o printing.o database.o dbdialog.o

# file dependencies
app.o : app.h frame.h
frame.o : frame.h dayview.h trussthumbview.h weekview.h trereader.h truss.h \
  daypickerdlg.h treatasdlg.h printing.h dbdialog.h
weekview.o : weekview.h dayview.h simpletruss.h floortruss.h trussthumbview.h \
  frame.h printing.h
dayview.o : dayview.h trussthumbview.h truss.h weekview.h frame.h printing.h
trussthumbview.o : trussthumbview.h bedviewdlg.h truss.h weekview.h dayview.h \
  frame.h printing.h dbdialog.h
bedviewdlg.o : bedviewdlg.h bedview.h truss.h
bedview.o : bedview.h truss.h
truss.o : truss.h simpletruss.h floortruss.h
simpletruss.o : simpletruss.h truss.h
floortruss.o : floortruss.h truss.h
trereader.o : trereader.h simpletruss.h floortruss.h truss.h
daypickerdlg.o : daypickerdlg.h
treatasdlg.o : treatasdlg.h trereader.h
printing.o : printing.h
database.o : database.h
dbdialog.o : dbdialog.h database.h

