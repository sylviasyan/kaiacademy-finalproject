
from tut.db import db
import os 
from dataclasses import dataclass, asdict
import json



def random_id():
    return os.urandom(16).hex()

@dataclass
class TestDB(db.Model):
    id  = db.Column(db.Integer, primary_key=True)
    foo: str = db.Column(db.String)
    bar: str = db.Column(db.String) 
    def toJSON(self):
        return asdict(self)

@dataclass
class whichOperatorRequestWhat(db.Model): # our commands
    id =  db.Column(db.Integer, primary_key=True)
    task_id : str = db.Column(db.String) #is the actual id to keep track of the tasks being done, the id the implant will be seeing
    implant_id : str = db.Column(db.String) # the implant it will be targeting
    cmd : str = db.Column(db.String) # specific command we send to implant          cmd == TASK
    args : str = db.Column(db.String) # args associated with cmd, it is a string that will be serialized by our gRPC proto buff

# we belive the class Tasks below is either the commands or jobs

@dataclass
class implantTasksThatAreDoneOrWIP(db.Model): # our jobs
    id =  db.Column(db.Integer, primary_key=True)
    task_id : str = db.Column(db.String) #is the actual id to keep track of the tasks being done, the id the implant will be seeing
    implant_id : str = db.Column(db.String) # the implant it will be targeting
    cmd : str = db.Column(db.String) # specific command we send to implant          cmd == TASK
    args : str = db.Column(db.String) # args associated with cmd, it is a string that will be serialized by our gRPC proto buff

@dataclass
class implantTasksThatAreDoneOrWIP(db.Model): # our clients
    id =  db.Column(db.Integer, primary_key=True)
    username : str = db.Column(db.String) # == username
    hostname : str = db.Column(db.String) # == computer name
    task_id : str = db.Column(db.String) #is the actual id to keep track of the tasks being done, the id the implant will be seeing
    implant_id : str = db.Column(db.String) # the implant it will be targeting
    cmd : str = db.Column(db.String) # specific command we send to implant          cmd == TASK
    args : str = db.Column(db.String) # args associated with cmd, it is a string that will be serialized by our gRPC proto buff
    clientID : str = db.Column(db.String)


@dataclass
class Implant(db.Model):
    id =  db.Column(db.Integer, primary_key=True)
    implant_id : str = db.Column(db.String)
    username : str = db.Column(db.String) # == username
    hostname : str = db.Column(db.String) # == computer name
    # add GUID, what is computers GUID?
    # add Integrity, what privileges do you have?
    # add Connecting IP address: what address did it connect from?
    # might not need to add here Session Key: after you negotiated a session key, store it per agent
    # add Sleep: how often does the agent check in? Andrea should have this value from implant
    # add Jitter: how random of a check in is it?
    # add First Seen: when did the agent first check in?
    # add Last Seen: when was the last time you saw the agent?
    # add Expected Check in: When should you expect to see the agent again?

    os_type : str = db.Column(db.String)
    os_name : str =db.Column(db.String)
    os_version :str  = db.Column(db.String)

    
    # todo add session crypto 
    def toJSON(self):
        return asdict(self)

CREATED = "CREATED"
TASKED = "TASKED"
COMPLETE = "COMPLETE"
ERROR = "ERROR"

@dataclass
class Task(db.Model): # jobs (?)
    id =  db.Column(db.Integer, primary_key=True) # underlying id in back end db
    task_id : str = db.Column(db.String) #is the actual id to keep track of the tasks being done, the id the implant will be seeing
    implant_id : str = db.Column(db.String) # the implant it will be targeting
    cmd : str = db.Column(db.String) # specific command we send to implant          cmd == TASK
    args : str = db.Column(db.String) # args associated with cmd, it is a string that will be serialized by our gRPC proto buff
    
    # maybe add timestamp

    # when something happened 
    # what happened 
    # what effect did it have
    # who did it
    status : str =  db.Column(db.String) # might wanna play with data types
    
    # @TODO it would be nice to know when the job was created! when it was pulled down and when it was executed
    ##timestamp =  
    def toJSON(self):
        return asdict(self)
