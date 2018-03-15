# Project 1 README:

*  Download Osmosis : follow the instruction below
*  wget https://bretth.dev.openstreetmap.org/osmosis-build/osmosis-latest.tgz
mkdir osmosis
mv osmosis-latest.tgz osmosis
cd osmosis
tar xvfz osmosis-latest.tgz
rm osmosis-latest.tgz
chmod a+x bin/osmosis
bin/osmosis

*  Download the submission 
*  Download boundaries of Edmonton (CoE_20Corporate_20Boundary_20_Tableau_.csv) from: https://data.edmonton.ca/Administrative/City-of-Edmonton-Corporate-Boundary-Tableau-/7mw4-eum4 , save it to unit1.
*  Download alberta-latest.osm.pdf from  https://download.geofabrik.de/north-america/canada.html
*  Open terminal in that folder
*  Run find_boundaries.py (python find_boundaries.py)
*  Find the left, right, top, bottom of boundaries，
*  Goto your Osmosis directory in terminal
*  Extracting map data with Osmosis: bin/osmosis osmosis --read-pbf [your path] --bounding-box bottom=53.3954049 left=-113.7138802 top=53.71591885 right=-113.2715238 --write-xml edmonton.osm
*  A backup of edmonton.osm can download from my google drive.
*  cut the result file edmonton.osm to the unit1 folder
*  Make sure you delete the test.db before running the ReadXMLFile.py
*  Loading OSM data into a SQLite database: python ReadXMLFile.py
*  The database Name is test.db
*  Make sure you let foreign key on 

