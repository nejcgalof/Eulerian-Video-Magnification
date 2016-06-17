# Eulerian-Video-Magnification
Comments and user interface in slovenian language (seminar work)

QT/C++/OpenCV
Documentation and concept of idea: [MIT research](http://people.csail.mit.edu/mrub/vidmag/)

##Documentation in Slovenian language:

###UVOD
Veliko statičnih prizorov vsebuje nekakšne spremembe, ki so nevidni prostim očesom. Možno je z različnimi pristopi izvleči te majhne spremembe iz videoposnetka. Eden izmed načinov je tudi algoritem Eulerian motion magnification, ki so ga razvili na MIT. Začetki raziskovanja na tem področju segajo v leto 2005, dokumentacija pa je bila izdana leta 2012. 

###ALGORITEM
Algoritem deluje tako, da vzamemo vsako sliko iz posnetka in jo obdelamo po algoritmu.
Imamo 7 korakov, kateri sliko spremenijo do te mere, da poudarimo gibanje. 

Prvi korak je pretvorba v drugi barvni prostor. V mojem primeru sem pretvoril iz RGB v LAB barvni prostor.  LAB barvni prostor ima 3 dimenzije:  svetlost ter A in b pravokotni osi 3D prostora. A os ima barvo od zelene do rdeče, B os pa od modre do rumene. Svetlost nastavlja parameter z vrednostjo od 0-100,kateri še posebej poskrbi za efekt odbite in prepuščene svetlobe. Barva se transformira tako, da se približa človeški zaznavi barve. Ta barvni model poskrbi za takšno zaznavanje tako, da če sta si dve barvi podobni, potem sta si tudi blizu v barvnem prostoru. 

![alt tag](https://8b474631b5f90854d5d5-29274c1ddc54cee4fa6f1b98374e5715.ssl.cf1.rackcdn.com/cie-lab.jpeg)

Iz dokumentacije povzeta slika prikazuje postopek algoritma. Tukaj je zelo enostavno na sklope razdeljena shema. Kot vidimo, sta v prvem delu dva koraka prikazana: Spatial decomposition in temporal processing. V mojem primeru sta to drugi in tretji korak algoritma.

![alt tag](http://www.extremetech.com/wp-content/uploads/2013/02/eulerian-video-magnification-diagram.jpg)

Drugi korak je prostorsko filtriranje oziroma angleško spatial filtering. Uporablja se za izboljšanje komaj opaznih signalov. Deluje na principu slikovnih piramid.
Cilj je zgraditi Laplacovo piramido. Ker je to prostorsko filtriranje, delamo po posameznih frekvenčnih prostorih. Piramide pravzaprav predstavijo sliko kot zaporedje slik, kjer vsaka slika vsebuje nižje frekvence. 
Če želimo ustvariti Laplacovo piramido, moramo najprej konstruirati Gaussovo piramido.
Konstruiramo jo tako, da sliko zgladimo z Gaussovim filtrom in jo zmanjšamo za polovico. Postopek ponovimo na zmanjšani sliki. To delamo tako dolgo, kolikor želimo imeti plasti v piramidi. V mojem programu sem uporabil 4 plasti. Velikost Gaussovega jedra je fiksna, po vsakem koraku (kolikor je plasti) pa zavržemo vsak drugi piksel. 
Ker z vsakim filtriranjem odstranimo del frekvenc lahko z odštevanjem zaporednih nivojev dobimo frekvenčne pasove slike ali drugače imenovano Laplacovo piramido.
Slike iz Gaussove piramide je potrebno pred odštevanjem skalirati nazaj na ustrezno velikost.
Značilnost Laplacove piramide je, da iz vsote vseh njenih nivojev dobimo originalno sliko.

![alt tag](http://docs.opencv.org/2.4/_images/Pyramids_Tutorial_Pyramid_Theory.png)

Gaussove piramide lahko tvorimo z uporabo Opencv funkcije pyrDown(). 
Rezultat te funkcije pri 4 plasteh piramide je prikazan na spodnji sliki.

![alt tag](http://docs.opencv.org/master/messipyr.jpg)

Funkcija pyrUp() poskrbi za povečanje resolucije in zameglitve(blur). To funkcijo potrebujemo za gradnjo Laplacove piramide, oziroma za nadaljnje odštevanje slik iz Gaussove piramide.

![alt tag](http://docs.opencv.org/master/messiup.jpg)

Za tvorbo Laplacove piramide iz Gaussove ne obstaja funkcije, ker je ne potrebujemo saj na koncu le odštevamo slike. 

![alt tag](http://docs.opencv.org/master/lap.jpg)

Nato sledi tretji korak, kjer bomo poudarjali gibanja, s časovnim filtriranjem (temporal filtering). To naredimo nad vsako plastjo Laplacove piramide. Filtriramo s pomočjo IIR(infinite impulse response) filtrov. Namesto IIR bi lahko uporabili tudi Ideal filter. 
Izračunamo zgornji in spodnji »lowpass« pas (lowpass1 in lowpass2) s pomočjo IIR filtra.  Nato izračunamo razliko, katero bomo uporabljali kasneje. 
Nato poračunamo nekaj parametrov za nadaljnjo uporabo. Najprej poračunamo parameter delta.
Ta poskrbi za povečanje vsakega prostorskega (spatial) frekvenčnega pasa.  Vpliva na spremembo alpha, katero bomo potrebovali kasneje. Določimo tudi exaggeration(okrepitveni) faktor, kateri okrepi alpha nad mejo, to pa vpliva na boljšo vizualizacijo. Izračunamo pa tudi reprezentativno valovno dolžino lambda, za najnižjo plast Laplacove piramide. 
V tretjem koraku je še potrebno dokončno ojačati vsako piramidno plast. Temu koraku se reče Amplification. 
Ko plast okrepimo, zmanjšamo lambdo za polovico in gremo na novo plast.
Samo krepitev izvajamo nad plastjo v svoji funkciji. Izračunamo spremenjen alpha  s pomočjo parametrov alpha in delta. Prav tako pomnožimo še z exeggeration faktorjem. Iz določenih razlogov ne spreminjamo robni plasti (najvišjo in najnižjo). Ostale plasti pa pomnožimo z minimalnim parametrom: alpha ali spremenjenim alpha. Alpha parameter podamo kot vhodni parameter.
Tako smo zaključili tretji korak. Sedaj smo naredili spatial in temporal filtriranje.

V naslednjem, četrtem koraku je potrebno narediti rekonstrukcijo gibanja iz Laplacove piramide. To shranimo v motion. Rekonstuiramo z funkcijo pyrUp, katera poskrbi za Up-sample.

V petem koraku nastopi attenuate (oblažitev/dušenje).  To storimo tako, da iz slike piramide izvedemo kromatično blaženje nad dvema kanaloma. Se pravi iz LAB spremenimo oziroma zmanjšamo le A in B kanala tako, da pomnožimo s parametrom chromAttenauation, katerega vnesemo kot vhodni parameter.

V šestem koraku le združimo originalno sliko in sliko ojačenega gibanja. To naredimo tako, da seštejemo obe sliki. 

V sedmem koraku pa pretvorimo rezultat iz LAB barvnega prostora nazaj v RGB.

###VHODNI PARAMETRI
Kot smo že videli v algoritmu imamo določene parametre podane preden zaženemo algoritem. Rezultat obdelanega videa je zelo odvisen od podanih vrednostih vhodnih parametrov. 
Alpha je faktor ojačanja. Večja kot je vrednost, več gibanja ojača.
Lambda_c v literaturi poimenujejo tudi Cutoff Wavelength. V slovenščini bi rekli temu kritična valovna dolžina. Naloga tega parametra je, da zmanjšuje hitro gibanje (kako občutljivo je na hitro gibanje).  Deluje na spatial (prostorski) frekvenci. 
Nizka in visoka frekvenca določata frekvenčni razpon, s katerim želimo delati. Na tem pasu se posebej okrepijo počasni premiki bolj kot hitri. Deluje na temporal (časovni) frekvenci.
Kromatično slabljenje deluje na 2 kanala, opisano v algoritmu. Večja kot je vrednost, bolj so ojačeni kromatični kanali, takrat dobimo bolj barvne gibe. 

V algoritmu imamo tudi fiksno določene parametre. 
Exaggeration faktor ki poskrbi za boljšo vizualizacijo tako, da okrepi alfo nad mejo. 
Leveli piramide oziroma število plasti piramide.

###REZULTATI
Video odprem, zaženem algoritem in obdelan video shranim. Na posnetkih lahko vidimo razlike. Ugotovil sem, da je zelo odvisno od nastavljenih posnetkov in od same kvalitete posnetka. Prav tako določene stvari ni primerno ojačiti, saj dobimo zelo čuden rezultat.

##Documentation in English language:
Coming soon with comments

##License
[MIT research](http://people.csail.mit.edu/mrub/vidmag/)

[Pyramids-Slovenian](https://ucilnica.fri.uni-lj.si/pluginfile.php/3941/mod_resource/content/6/MS_manipulacijaSlik.pdf)

[Code example 1](http://www.cs.cmu.edu/afs/cs.cmu.edu/academic/class/15463-f12/www/proj2/www/ygu1/)

[Code example 2](https://www.cg.tuwien.ac.at/courses/Visualisierung2/HallOfFame/2015/Spreenix/Homepage/html/lpiir.html)

[Code example 3](https://github.com/wzpan/QtEVM)

[Code example 4](https://github.com/tschnz/Realtime-Video-Magnification)

[OpenCV documentation 1](http://opencv-python-tutroals.readthedocs.io/en/latest/)

[OpenCV documentation 2](http://docs.opencv.org/3.1.0/#gsc.tab=0)

And thanks for MIT for sharing their MATLAB code
