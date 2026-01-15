#include "mainwindow.h" 
 #include <QApplication> 
 #include <QDebug> 
 
 extern "C" { 
     int rustalk_add(int a, int b); 
 } 
 
 int main(int argc, char *argv[]) 
 { 
     QApplication a(argc, argv); 
 
     int result = rustalk_add(2, 3); 
     qDebug() << "rustalk_add(2,3) =" << result; 
 
     MainWindow w; 
     w.show(); 
 
     return a.exec(); 
 }
