import csv
import sys

#increase the field size
csv.field_size_limit(sys.maxsize)

latitude=[]
longitude=[]

#read the each line of data in dictionary
with open('CoE_20Corporate_20Boundary_20_Tableau_.csv') as csvfile:
    f = csv.DictReader(csvfile)
    for data in f:
    	#print(data)
    	#extract each pairs of latitude and longitude into two lists
    	latitude.append(float(data["LATITUDE"]))
    	longitude.append(float(data["LONGITUDE"]))

#find the max and min values of latitude and longitude
left=min(longitude)
right=max(longitude)
bottom=min(latitude)
top=max(latitude)

print("left: ", left)
print("right: ", right)
print("bottom: ", bottom)
print("top: ", top)