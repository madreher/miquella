from typing import List
from sqlalchemy import String, create_engine, PickleType, select, update
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, Session 
from sqlalchemy.ext.mutable import MutableList

import uuid

################################ DATABASE Model ###################################
# Declare the data model for the job
class Base(DeclarativeBase):
    pass

class Job(Base):
    __tablename__ = "jobs_table"

    jobID: Mapped[str] = mapped_column(String(30), primary_key=True)
    sceneID: Mapped[int]
    nSamples: Mapped[int]
    freqOutout: Mapped[int]
    samples: Mapped[list[int]] = mapped_column(MutableList.as_mutable(PickleType))
    images:Mapped[list[str]] = mapped_column(MutableList.as_mutable(PickleType))
    status: Mapped[str]

    def __repr__(self) -> str:
        return f"Job(jobID={self.jobID!r}, sceneID={self.sceneID!r}, nSamples={self.nSamples!r}, freqOutput={self.freqOutout!r}, samples={self.samples!r}, images={self.images!r}, status={self.status!r})"

    def toJobRequestDict(self) -> dict:
        result = {}
        result["jobID"] = self.jobID
        result["sceneID"] = self.sceneID
        result["nSamples"] = self.nSamples
        result["freqOutput"] = self.freqOutout

        return result

    def toJobStatusDict(self) -> dict:
        result = {}
        result["jobID"] = self.jobID
        result["sceneID"] = self.sceneID
        result["nSamples"] = self.nSamples
        result["freqOutput"] = self.freqOutout
        if len(self.samples) == 0:
            result["lastSample"] = 0
            result["lastImage"] = ""
        else:
            result["lastSample"] = self.samples[-1]
            result["lastImage"] = self.images[-1]
        result["status"] = self.status

        return result
################################ DATABASE Model ###################################
        
# Class handler for the database 
class JobDatabase:
    def __init__(self, databasePath:str="sqlite://", echo:bool=False, createDatabase:bool=True) -> None:

        # Create the engine 
        self.engine = create_engine(databasePath, echo=echo)

        # Create the tables if required 
        if createDatabase:
            Base.metadata.create_all(self.engine)

        # Open a session which will stay open as long as the oject stays alive
        self.session = Session(self.engine)

    def addJob(self, sceneID:int=3, nSamples:int=1000, freqOutput:int=50) -> str:
        newJob = Job()
        newJob.jobID = str(uuid.uuid4())
        newJob.sceneID = sceneID
        newJob.nSamples = nSamples
        newJob.freqOutout = freqOutput
        newJob.samples = []
        newJob.images = []
        newJob.status = "PENDING"
        self.session.add_all([newJob])
        self.session.commit()

        return newJob.jobID
    
    def getRenderingJobForServer(self) -> dict:
        # Query the db
        stmt = select(Job).where(Job.status == "PENDING")
        jobs = self.session.execute(stmt)

        firstJob = jobs.first()

        # Check if we have a job
        if firstJob is None:
            return {}
        else:
            # Update the job to become running and commit
            firstJob[0].status = "RUNNING"
            self.session.commit()

            # Sending the job to the server
            return firstJob[0].toJobRequestDict()
        
    def addSampleToJob(self, jobID:str, filePath:str, lastSample:int) -> dict:
        # Select the job
        stmt = select(Job).where(Job.jobID == jobID)
        jobs = self.session.execute(stmt)

        # Dev warning: first() cancel the rest of the potential row in the result
        # and close the result. Can't call first() again on jobs after this
        firstJob = jobs.first()
        
        if firstJob is None:
            # The job either doesn't exist or have been deleted. 
            result = {}
            result["status"] = "REMOVED"
            return result
        
        firstJob[0].samples.append(lastSample)
        firstJob[0].images.append(filePath)

        if lastSample == firstJob[0].nSamples:
            firstJob[0].status = "COMPLETED"
        self.session.commit()

        # Return the status of the job so that the server knows if it can continue
        result = {}
        result["status"] = firstJob[0].status
        return result
    
    def cancelJob(self, jobID:str) -> dict:
        # Select the job
        stmt = select(Job).where(Job.jobID == jobID)
        jobs = self.session.execute(stmt)

        # Dev warning: first() cancel the rest of the potential row in the result
        # and close the result. Can't call first() again on jobs after this
        firstJob = jobs.first()
        
        if firstJob is None:
            result = {
                "error" : "Job does not exist."
            }
            return result
        
        if firstJob[0].status == "COMPLETED":
            result = {
                "error" : "Job is already completed. Nothing to "
            }
            return result
        
        # Updating the status of the job.
        firstJob[0].status = "CANCELED"
        self.session.commit()

        result = { "status" : "CANCELED"}
        return result
    
    def removeJob(self, jobID:str):
        stmt = select(Job).where(Job.jobID == jobID)
        jobs = self.session.execute(stmt)

        # Dev warning: first() cancel the rest of the potential row in the result
        # and close the result. Can't call first() again on jobs after this
        firstJob = jobs.first()
        
        if firstJob is None:
            result = {
                "error" : "Job does not exist."
            }
            return result

        self.session.delete(firstJob[0])
        self.session.commit()

        result = {
            "status" : "REMOVED"
        }
        return result
    
    def getLastSampleFromJob(self, jobID:str, ) -> dict:
        stmt = select(Job).where(Job.jobID == jobID)
        jobs = self.session.execute(stmt)
        
        # Dev warning: first() cancel the rest of the potential row in the result
        # and close the result. Can't call first() again on jobs after this
        firstJob = jobs.first()

        result = {}
        
        if firstJob is None:
            result = {
                "error" : "Job does not exist."
            }
        elif firstJob[0].status == "PENDING":
            result = {
                "error" : "Job is not running yet."
            }
        elif len(firstJob[0].samples) == 0:
            result = {
                "lastSample" : 0,
                "image" : "",
                "status" : firstJob[0].status
            }
        else:
            result = {
                "lastSample" : firstJob[0].samples[-1],
                "image" : firstJob[0].images[-1],
                "status" : firstJob[0].status
            }
        return result
    
    def getCompleteListOfJobs(self) -> dict:
        stmt = select(Job)
        jobs = self.session.execute(stmt)

        result = {}
        result["jobs"] = []

        for job in jobs:
            result["jobs"].append(job[0].toJobStatusDict())
        return result