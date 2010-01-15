CMDDB sorting:
sed -e 's/\t\([0-9][0-9]\)\./\t0\1./g' <cmddb.tsv.in |
sort -u |
sed -e 's/\t0\([0-9][0-9]\)\./\t\1./g' >cmddb.tsv.out

Credits:
http://sourceforge.net/projects/garmin-img/
http://libgarmin.sourceforge.net/
http://ati.land.cz/
http://svn.parabola.me.uk/display/trunk/doc/nod.txt
http://wiki.openstreetmap.org/wiki/OSM_Map_On_Garmin/MDR_Subfile_Format
