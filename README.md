# Molekulák OpenGL-ben

A projekt a számítógépes grafika tárgyon kiadott házifeladatok egyikét valósítja meg. A feladat megvalósításához kiadott keretprogram megtalálható [itt](https://cg.iit.bme.hu/portal/content/hazi-feladatok).

## Leírás

A projekt lényege, geometriai transzformációk használatával, véletlenszerűen generált 2 molekula egymásra kifejtett kölcsönhatásának fizikai szimulációja.

- Minden molekula merev, random fagráf topológiájú

- A gráf csúcsait az atomok alkotják (minimum 2, maximum 8)

-  Minden atomnak randomizált a töltése úgy, hogy a molekulán belüli össztöltés semleges legyen

- Az atomok rendelkeznek tömeggel és intenzitással (megjelenítéskor a színek a töltéssel arányos intenzitásúak).

- Az atomok között Coulomb erő lép fel, mely hatására a molekulák elkezdenek mozogni/forogni

- Az atomokra a sebességgel arányos közegellenállás érvényesül

A fizikai szimuláció az euklidészi síkon van megvalósítva, mely a hiperbólikus síkra van leképezve és Beltrami-Poincaré leképzéssel megjelenítve a képernyőn.

- Az "s, d, x, e" billentyűkkel az euklidészi síkot el lehet tolni jobbra, balra, le és fel

- space billentyű lenyomására új molekulák generálódnak

<p align="center">
    <img src="/1.png">
    <img src="/2.png">
</p>