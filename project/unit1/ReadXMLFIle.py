
import xml.etree.ElementTree as ET  
import sqlite3
import time
import sys


def createTrigger(c):
    c.execute('''drop trigger if exists check_insert;''')
    c.execute('''drop trigger if exists check_delete;''')
    c.execute('''drop trigger if exists check_delete_start;''')
    c.execute('''drop trigger if exists check_insert_true;''')
    c.execute('''drop trigger if exists check_insert_false;''')
    c.execute('''drop trigger if exists check_update_true;''')
    c.execute('''drop trigger if exists check_update_false;''')
    c.execute('''drop trigger if exists check_delete_true;''')
    c.execute('''drop trigger if exists check_delete_false;''')
    c.execute('''drop trigger if exists check_way_delete;''')

    c.execute(''' CREATE TRIGGER check_way_delete
    	          AFTER DELETE ON way
    	          BEGIN
    	          DELETE FROM waypoint where wayid = old.id;
    	          end; ''')
    c.execute('''CREATE TRIGGER check_insert
                BEFORE INSERT ON waypoint
                WHEN ((SELECT MAX(ordinal) FROM waypoint WHERE new.wayid=wayid)+1)<>new.ordinal
                BEGIN
                SELECT RAISE(ABORT, "Invalid ordinal!");
                END;''')

    c.execute('''CREATE TRIGGER  check_delete 
                 BEFORE DELETE ON waypoint
                 WHEN old.ordinal<>(SELECT MAX(ordinal) FROM waypoint WHERE old.wayid=wayid) AND old.ordinal<>(SELECT MIN(ordinal) FROM waypoint WHERE old.wayid=wayid)
                 BEGIN
                 DELETE FROM way WHERE id=old.wayid;
                 END;''')

    c.execute('''CREATE TRIGGER  check_delete_start 
                 BEFORE DELETE ON waypoint
                 WHEN old.ordinal=0
                 BEGIN
                 DELETE FROM waypoint WHERE wayid=old.wayid and ordinal=old.ordinal;
                 UPDATE waypoint SET ordinal=(ordinal-1) where wayid=old.wayid;
                 END;''')

    c.execute('''CREATE TRIGGER check_insert_true
                AFTER INSERT ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE new.wayid=wayid))
                =(SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE new.wayid=wayid))
                BEGIN
                UPDATE way SET closed=1 WHERE new.wayid = way.id;
                END;''')

    c.execute('''CREATE TRIGGER check_update_true
                AFTER UPDATE ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE new.wayid=wayid))
                =(SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE new.wayid=wayid))
                BEGIN
                UPDATE way SET closed=1 WHERE new.wayid = way.id;
                END;''')

    c.execute('''CREATE TRIGGER check_delete_true
                AFTER DELETE ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE old.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE old.wayid=wayid))
                =(SELECT nodeid FROM waypoint WHERE old.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE old.wayid=wayid))
                BEGIN
                UPDATE way SET closed=1 WHERE old.wayid = way.id;
                END;''')

    c.execute('''CREATE TRIGGER check_insert_false
                AFTER INSERT ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE new.wayid=wayid))
                <>(SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE new.wayid=wayid))
                BEGIN 
                UPDATE way SET closed=0 WHERE new.wayid = way.id;
                END;''')

    c.execute('''CREATE TRIGGER check_update_false
                AFTER UPDATE ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE new.wayid=wayid))
                <>(SELECT nodeid FROM waypoint WHERE new.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE new.wayid=wayid))
                BEGIN 
                UPDATE way SET closed=0 WHERE new.wayid = way.id;
                END;''')

    c.execute('''CREATE TRIGGER check_delete_false
                AFTER DELETE ON waypoint
                WHEN (SELECT nodeid FROM waypoint WHERE old.wayid=wayid AND ordinal=(SELECT MAX(ordinal) FROM waypoint WHERE old.wayid=wayid))
                <>(SELECT nodeid FROM waypoint WHERE old.wayid=wayid AND ordinal=(SELECT MIN(ordinal) FROM waypoint WHERE old.wayid=wayid))
                BEGIN 
                UPDATE way SET closed=0 WHERE old.wayid = way.id;
                END;''')
   
    print "Trigger created successfully"
# This function init and create the sqlite3 tables
def creatTable(c):
    c.execute(''' DROP TABLE IF EXISTS node''')
    c.execute(''' DROP TABLE IF EXISTS nodetag''')
    c.execute(''' DROP TABLE IF EXISTS waytag''')
    c.execute(''' DROP TABLE IF EXISTS way''')
    c.execute(''' DROP TABLE IF EXISTS waypoint''')
    c.execute('''CREATE TABLE node 
           ( id integer PRIMARY KEY,
             lat float NOT NULL,
             lon float NOT NULL);''')
    c.execute('''CREATE TABLE nodetag 
         (id integer,
             k text,
             v text,
            FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE );''') 
    
    c.execute('''CREATE TABLE waytag 
         (id integer,
             k text,
             v text,
            FOREIGN KEY(id) REFERENCES node(id) ON DELETE CASCADE );''')       
    c.execute('''CREATE TABLE way
         (  id integer PRIMARY KEY,
             closed boolean);''')     

    c.execute('''CREATE TABLE waypoint 
         (wayid integer,
             ordinal integer,
             nodeid integer,
            FOREIGN KEY(wayid) REFERENCES way(id)
            FOREIGN KEY(nodeid) REFERENCES node(id) ON DELETE CASCADE);''')   
    print "Table created successfully";

# This function insert values into the table base on the given node ( tree structure node)

def handleTag(tag,c,tagtype,tagid):
    if tagtype == 'nodetag':
        ID = tagid
        k = tag.get('k')
        v = tag.get('v')        
        c.execute("insert into nodetag (id,k,v) values (?,?,?)",
                  (ID,k,v)) 
    elif tagtype == 'waytag':
        ID = tagid
        k = tag.get('k')
        v = tag.get('v')        
        c.execute("insert into waytag (id,k,v) values (?,?,?)",
                  (ID,k,v))        
        
def handleNode(root,c):
 
    ID = root.get('id')
    lat = root.get('lat')
    lon = root.get('lon')
    c.execute("insert into node (id,lat,lon) values (?,?,?)",
               (ID,lat,lon))          
            

def handleWayPoint(root,c,wayid,nodeidList):
    ordinal = len(nodeidList)
    nodeid = root.get('ref')
    nodeidList.append(nodeid)
    c.execute("insert into waypoint (wayid,ordinal,nodeid) values (?,?,?)",
               (wayid,ordinal,nodeid))  
    
def handleWay(root,c,nodeidList):
    
    ID = root.get('id')
    c.execute("insert into way (id,closed) values (?,?)",
            (ID,(nodeidList[0] == nodeidList[-1])))       

if __name__ == "__main__":
    start = time.time()
  
    tagtype = None;
    tagid = None;
    conn = sqlite3.connect('test.db')
    print "Opened database successfully";
    c = conn.cursor()    
    creatTable(c)
    conn.commit()
    context = ET.iterparse('edmonton.osm', events=("start", "end"))
    context = iter(context)
    i = 0.0
    for event, elem in context:
        sys.stdout.write('\r')
        sys.stdout.write( "doing ....." + str(i/14241302.0) + "%")
        i+=1.0
        if event == "start" :
            if  elem.tag == 'node':
                tagtype = 'nodetag'
                tagid = elem.get('id')
            elif elem.tag == 'way':
                nodeidList = []
                tagtype = 'waytag'
                tagid = elem.get('id')
            
        if event == "end" :
            if elem.tag == 'node':
                handleNode(elem,c)
            elif elem.tag == 'tag':
                handleTag(elem,c,tagtype,tagid)
            elif elem.tag == 'nd':
                handleWayPoint(elem,c,tagid,nodeidList)
            elif elem.tag =='way':
                handleWay(elem,c,nodeidList)
            elem.clear()

    createTrigger(c)
    conn.commit()
    conn.close() 

    end = time.time()
    
    elapsed = end - start      
    print "time elapsed: " + str(elapsed) + " seconds"