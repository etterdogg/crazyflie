crazyflie
=========
Här lägger allt planeringsrelaterat och uppdaterar vår projektplan.

projektplan
-----------

- lv 7 producera rapport
- lv 6 Mål: Få en dator att via kinekt hovra en crazyflie.
- lv 5 skriva reglering
- lv 5 Programera kinekt (opencv?)
- lv 4 Få kinekt att känna igen crazyflie
- lv 4 Möjliggöra för kinekt att se crazyflien
- lv 3 kunna koda crazyflien (python)
- lv 2 Lära oss flyga

arbetsfördelning
----------------
__Martin__
Prata OpenNI med Kinect och producera data att skicka vidare

__Niklas & Oskar__
Identifiera bollen med OpenCV och implementera regulatorn i python

__Viktor__
Prata med cf och skriva init-script (?)

arbetslogg
----------
Loggfil finns i ./LOGG.md

Mall för inlägg:

    ÅÅÅÅ-MM-DD : NAMN
    beskrivning

Frågor på det?


För att utveckla behövs följande paket:

OpenCv  https://help.ubuntu.com/community/OpenCV  

Freenect  http://openkinect.org/wiki/Getting_Started    skrolla ner till manual install  

eclipse med cdt plugin (help->install new software)  


För att fixa problem med att eclipse inte hittar funktioner och bibliotek  
i c++ compilerns include (högerklicka på projektet, välj preferences, C/C++ build -> settings -> gcc c++ compiler -> includes ) ska följande finnas med under include path  
/usr/local/include  

under gcc c++ linker->libraries skall följande finnas  
opencv_core  
freenect__sync  
frenect  
opencv_imgproc  
opencv_highgui  

i library search path  
/usr/local/lib  
/usr/local/lib64  



Om programmet inte hittar filer när det startar:  
skapa filen usr-loc-lib.conf med följande innehåll  

/usr/local/lib  
/usr/local/lib64  

och lägg i /etc/ld.so.conf/  
kör sedan   
sudo ldconfig  


taadaaaa
