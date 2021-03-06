import mysql.connector as mysql
import time


class  databaseConnection():

    dbConnection = 0
    db = 'dewi_experiments';                 
    def __init__(self):
        try:
            self.dbConnection = mysql.connect(host='localhost',user='root',db='dewi_experiments',buffered=True)
        except mysql.Error, e:
            self.dbConnection = mysql.connect(host='localhost',user='root',buffered=True)
            cursor = self.dbConnection.cursor()
            cursor.execute("CREATE DATABASE dewi_experiments DEFAULT CHARACTER SET 'utf8'")     
            self.dbConnection.database = 'dewi_experiments'
        try:            
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM experiments")
            for id in cursor:
                id
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE experiments (id INT NOT NULL, description TEXT NULL, date_time INT(11) NULL, PRIMARY KEY(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM settings")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE settings (id INT NOT NULL AUTO_INCREMENT, session_id INT NOT NULL, txPower TEXT NOT NULL, numberBursts INT NOT NULL, burstDuration INT NOT NULL, MSGPerBurst INT NOT NULL, PRIMARY KEY(id), FOREIGN KEY(session_id) REFERENCES dewi_experiments.experiments(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM latency")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE latency (id INT NOT NULL AUTO_INCREMENT, session_id INT NOT NULL,nodeID INT NOT NULL,timeslot INT NOT NULL, count INT NOT NULL, PRIMARY KEY(id), FOREIGN KEY(session_id) REFERENCES dewi_experiments.experiments(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM rxpackets")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE rxpackets (id INT NOT NULL AUTO_INCREMENT, session_id INT NOT NULL,nodeID INT NOT NULL,count INT NOT NULL, PRIMARY KEY(id), FOREIGN KEY(session_id) REFERENCES dewi_experiments.experiments(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM txpackets")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE txpackets (id INT NOT NULL AUTO_INCREMENT, session_id INT NOT NULL,nodeID INT NOT NULL,count INT NOT NULL, PRIMARY KEY(id), FOREIGN KEY(session_id) REFERENCES dewi_experiments.experiments(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM neighbours")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE neighbours (id INT NOT NULL AUTO_INCREMENT, session_id INT NOT NULL,parent INT NOT NULL,child INT NOT NULL,tier INT NOT NULL,colour INT NOT NULL, PRIMARY KEY(id), FOREIGN KEY(session_id) REFERENCES dewi_experiments.experiments(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT * FROM tempneighbours")
            for id in cursor:
                id
        
        except mysql.Error, e:
            try:
                self.dbConnection.cursor().execute("CREATE TABLE tempneighbours (id INT NOT NULL AUTO_INCREMENT,parent INT NOT NULL,child INT NOT NULL,tier INT NOT NULL,colour INT NOT NULL, PRIMARY KEY(id))")
            except IndexError:
                print "MySQL Error: %s" % str(e)
  
    
    def insertExperiment(self,id,description,date_time):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("INSERT INTO experiments (id,description,date_time) VALUES ({0},'{1}',{2})").format(id,description,date_time))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "insertExperiment"
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
  
    
    def insertSettings(self,id, session_id, txPower, numberBursts, burstDuration,MSGPerBurst):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("INSERT INTO settings(id, session_id, txPower, numberBursts, burstDuration,MSGPerBurst) VALUES ({0},{1},{2},{3},{4},{5})").format(id, session_id, txPower, numberBursts, burstDuration,MSGPerBurst))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "insertSettings"
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def insertLatency(self,id, session_id, nodeID,timeslot,count):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("SELECT * FROM latency WHERE session_id = {0} AND nodeID = {1} AND timeslot = {2}").format(session_id, nodeID,timeslot))
            uniqe = True
            for row in cursor:
                uniqe = False     
                
            if uniqe == False:
                return
            cursor.execute(("INSERT INTO latency(id, session_id, nodeID,timeslot,count) VALUES ({0},{1},{2},{3},{4})").format(id, session_id, nodeID,timeslot,count))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "insertLatency"
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def insertRxPackets(self,id, session_id, nodeID,count):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("SELECT * FROM rxpackets WHERE session_id = {0} AND nodeID = {1}").format(session_id, nodeID))
            uniqe = True
            for row in cursor:
                uniqe = False     
                
            if uniqe == False:
                return
            cursor.execute(("INSERT INTO rxpackets(id, session_id, nodeID,count) VALUES ({0},{1},{2},{3})").format(id, session_id, nodeID,count))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "insertRxPackets"
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def insertTxPackets(self,id, session_id, nodeID,count):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("SELECT * FROM txpackets WHERE session_id = {0} AND nodeID = {1}").format(session_id, nodeID))
            uniqe = True
            for row in cursor:
                uniqe = False     
                
            if uniqe == False:
                return
            cursor.execute(("INSERT INTO txpackets(id, session_id, nodeID,count) VALUES ({0},{1},{2},{3})").format(id, session_id, nodeID,count))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "insertTxPackets"
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def insertNeighbour(self,id,session_id, parent,child,tier,colour):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("INSERT INTO neighbours(id, session_id,parent,child,tier,colour) VALUES ({0},{1},{2},{3},{4},{5})").format(id,session_id, parent,child,tier,colour))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def insertTempNeighbour(self,id, parent,child,tier,colour):        
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("INSERT INTO tempneighbours(id, parent,child,tier,colour) VALUES ({0},{1},{2},{3},{4})").format(id,parent,child,tier,colour))
            self.dbConnection.commit()
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def getLastExperimentID(self):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute(("SELECT MAX(id) FROM experiments"))
            
            for id in cursor:
                if str(id[0]) == 'None':
                    return -1
                else:
                    return int(id[0])
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
        
        return 0
        
    def getAllNeighbours(self):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT parent,child,tier,colour FROM tempneighbours ORDER BY tier ASC")
            result = []
            for parent,child,tier,colour in cursor:
                result.append((hex(parent),hex(child),tier,colour))
            return result
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
        
        return 0
    
    def getLinkAddressList(self):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT DISTINCT parent AS Number FROM tempneighbours")
            result = []
            for parent in cursor:
                result.append(hex(parent[0]))
                
            
            cursor.execute("SELECT DISTINCT(child) AS Number FROM tempneighbours") 
            for child in cursor:
                uniqe=True
                for i in range(0,len(result)):
                    if result[i] == hex(child[0]):
                        uniqe=False
                        
                if uniqe==True:
                    result.append(hex(child[0]))  
                else:
                    print hex(child[0])
            return result
        except mysql.Error, e:
            print "getLinkAddressList"
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
        
        return 0
    
    def clearTempNeighbourList(self):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("TRUNCATE tempneighbours")
 
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
        
        return 0
        
    def saveNeighbours(self): 
           
        result = self.getAllNeighbours()
        id = self.getLastExperimentID()
        for i in range(0,len(result)):
            self.insertNeighbour(0,id,result[i][0],result[i][1],result[i][2],result[i][3])
            
    def getExperiments(self):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT description FROM experiments")
            descriptionRes = []
            for description in cursor:
                descriptionRes.append(str(description[0]))
            settingsRes = []
            cursor.execute("SELECT session_id,numberBursts,burstDuration,MSGPerBurst FROM settings") 
            for session_id,numberBursts,burstDuration,MSGPerBurst in cursor:
                settingsRes.append((session_id,numberBursts,burstDuration,MSGPerBurst))   

  
            completeRes = []
            for i in range(0,len(settingsRes)):
                completeRes.append((descriptionRes[i],settingsRes[i][0],settingsRes[i][1],settingsRes[i][2],settingsRes[i][3]))
                
            return completeRes
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)
                
    def getLatencyResult(self,session_id):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT timeslot,count FROM latency WHERE session_id={0} ORDER BY timeslot ASC".format(session_id))
            latencyRes = []
            for timeslot,count in cursor:
                latencyRes.append((int(timeslot),int(count)))
            return latencyRes
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)

    def getDescription(self,session_id):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT description FROM experiments WHERE id={0}".format(session_id))
            descriptionRes = []
            for description in cursor:
                print description
                descriptionRes.append(str(description[0]))
            return descriptionRes
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)


    def getTXPackets(self,session_id):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT count FROM txpackets WHERE session_id={0}".format(session_id))
            latencyRes = []
            for count in cursor:
                latencyRes.append(int(count[0]))
            return latencyRes
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)                
    def getRXPackets(self,session_id):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT count FROM rxpackets WHERE session_id={0}".format(session_id))
            latencyRes = []
            for count in cursor:
                latencyRes.append(int(count[0]))
            return latencyRes
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)

    def getRXPacketsForNode(self,session_id,addr):
        try:
            cursor = self.dbConnection.cursor()
            cursor.execute("SELECT count FROM rxpackets WHERE session_id={0} AND nodeID={1}".format(session_id,addr))
            latencyRes = []
            for count in cursor:
                return count[0]
        except mysql.Error, e:
            try:
                print "MySQL Error [%d]: %s" % (e.args[0], e.args[1])
            except IndexError:
                print "MySQL Error: %s" % str(e)

    def returnHex(self,text):
        try:
            return hex(text)
        except TypeError, e:
            print e;

# dewi = databaseConnection()
#
# print dewi.getDescription(8)