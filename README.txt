gimgtools is a set of command-line tools to examine and manipulate Garmin IMG
(the map format) files. The tools are:

* gimgunlock: Unlock a locked map so that it can be used on ALL devices. There
    is no need to specify your device ID or map keys. It works by decrypting
    the TRE section. The decrypting key is the same for all maps. If you like
    Garmin, please buy their map!
  Usage: gimgunlock map.img
* gimgxor: Some maps have been scrambled by a trival XOR algorithm. They do
    not work with other gimg* tools. You can use gimgxor to unscramble them.
  Usage: gimgxor map.img
=== The following tools are for reverse engineering only ===
* gimginfo: Print information of the map.
  Usage: gimginfo map.img
* gimgextract: Extract the IMG sections.
  Usage: gimgextract map.img
* gimgch: Hexdump and compare section header of two or more IMGs. Extremely
    useful for reverse engineering the file format.
  Usage: gimgch [-w columns] [-m max_sf_per_img] [-s pattern] map1.img map2.img ...

Other than that, I have done some research on the deviation of China map
coordinates. Though there is no concrete result, you can follow it in this
article
http://wuyongzheng.wordpress.com/2010/01/22/china-map-deviation-as-a-regression-problem/
My ultimate goal is to change the coordinates in place, i.e. change individual
bytes without recompiling the map.  The cmdc tool is to generate the deviation
table. The gimgfixcmd tool fixes the deviation of a map.

Bug Report:
Please post it to google code http://code.google.com/p/gimgtools/. If you can
fix it, you can do so in github and let me know.

Credits:
Most of the reverse engineering on IMG format was done by other pepole (listed
below). I just read their docs and code. The reverse engineering of the
unlocking algorithm and the GMP section (aka the NT format by Garmin) are my
work.

http://sourceforge.net/projects/garmin-img/
http://libgarmin.sourceforge.net/
http://ati.land.cz/
http://svn.parabola.me.uk/display/trunk/doc/nod.txt
http://wiki.openstreetmap.org/wiki/OSM_Map_On_Garmin/MDR_Subfile_Format
