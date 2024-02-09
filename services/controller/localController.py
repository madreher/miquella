#!/usr/bin/env python3

from fastapi import FastAPI, Form, Request
from fastapi.responses import JSONResponse,Response
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi import FastAPI, File, UploadFile

from typing import List
from sqlalchemy import String, create_engine, PickleType, select, update
from sqlalchemy.orm import DeclarativeBase, Mapped, mapped_column, Session 
from sqlalchemy.ext.mutable import MutableList



import uvicorn
import os
import uuid

################################ DATABASE ###################################
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
# Create the engine which is used to connect to the database
engine = create_engine("sqlite://", echo=True)

# Create the table
# IMPORTANT: For now, the assumption is that the database is alive only
# as long as the controller lives. This will need to change when switching 
# to a permanent form of database.
Base.metadata.create_all(engine)

# Open a session and keep it open 
session = Session(engine)
################################ DATABASE ###################################

################################ WEBSERVER ###################################
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")
################################ WEBSERVER ###################################


#pendingJobDB = []

#runningJobDB = {}

CONTROLLER_FOLDER = os.path.join(os.environ["HOME"], ".miquella")
SAMPLE_FOLDER = os.path.join(CONTROLLER_FOLDER, "samples")




@app.post("/submit")
async def create_rendering_job(sceneID : int = 3, nSamples : int = 1000, freqOutput : int = 50):
    '''
        Send a query to perform a rendering task.
    '''
    jobId = str(uuid.uuid4())
    #pendingJobDB.append(
    #    {
    #        "jobID" : jobId, 
    #        "sceneID" : sceneID, 
    #        "nSamples" : nSamples, 
    #        "freqOutput" : freqOutput,
    #        "samples" : [],
    #        "images" : [],
    #        "status" : "PENDING"
    #    })
    
    # Add the job to the databse
    newJob = Job()
    newJob.jobID = jobId
    newJob.sceneID = sceneID
    newJob.nSamples = nSamples
    newJob.freqOutout = freqOutput
    newJob.samples = []
    newJob.images = []
    newJob.status = "PENDING"
    session.add_all([newJob])
    session.commit()

    return Response(content=jobId, media_type="text/html")

@app.post("/requestJob")
async def provision_job():
    '''
        Send a rendering job to a server
    '''

    # Query the db
    stmt = select(Job).where(Job.status == "PENDING")
    jobs = session.execute(stmt)

    firstJob = jobs.first()

    # Check if we have a job
    if firstJob is None:
        return JSONResponse(content={})
    else:
        # Update the job to become running and commit
        firstJob[0].status = "RUNNING"
        session.commit()

        # Sending the job to the server
        return JSONResponse(content=firstJob[0].toJobRequestDict())
    for job in jobs:
        print(job[0].jobID)
        print(job[0].images)

        # Update the the job
        print("Attempt at updating")
        job[0].status = "RUNNING"
        #updateCmd = 
        session.commit()

        updateSelect = select(Job).where(Job.status == "RUNNING")
        newJobs = session.execute(updateSelect)
        for job2 in newJobs:
            print(job2)
        print("End update")

    if len(pendingJobDB) > 0:
        jobID = pendingJobDB[0]["jobID"]
        runningJobDB[jobID] = pendingJobDB[0]
        runningJobDB[jobID]["status"] = "RUNNING"
        del pendingJobDB[0]

        # Query the db
        stmt = select(Job).where(Job.status == "PENDING")
        jobs = session.execute(stmt)
        for job in jobs:
            print(job[0].jobID)
            print(job[0].images)

            # Update the the job
            print("Attempt at updating")
            job[0].status = "RUNNING"
            #updateCmd = 
            session.commit()

            updateSelect = select(Job).where(Job.status == "RUNNING")
            newJobs = session.execute(updateSelect)
            for job2 in newJobs:
                print(job2)
            print("End update")

        return JSONResponse(content=runningJobDB[jobID])
    else:
        return JSONResponse(content={})

@app.post("/updateLocalJobExec")
async def updateLocalJobExec(jobID : str, filePath : str, lastSample : int):
    '''
        Update the runningJobDB with the last output done by a worker.
    '''
    #if jobID not in runningJobDB:
    #    raise RuntimeError("Received an unknown JobID.")
    
    #runningJobDB[jobID]["samples"].append(lastSample)
    #runningJobDB[jobID]["images"].append(filePath)
    #if lastSample == runningJobDB[jobID]["nSamples"]:
    #    runningJobDB[jobID]["status"] = "COMPLETED"

    # Select the job
    stmt = select(Job).where(Job.jobID == jobID)
    jobs = session.execute(stmt)

    # Dev warning: first() cancel the rest of the potential row in the result
    # and close the result. Can't call first() again on jobs after this
    firstJob = jobs.first()
    
    if firstJob is None:
        raise RuntimeError("Received an unknown JobID")
    
    firstJob[0].samples.append(lastSample)
    firstJob[0].images.append(filePath)
    if lastSample == firstJob[0].nSamples:
        firstJob[0].status = "COMPLETED"
    session.commit()

    # Check the result
    #jobs2 = session.execute(stmt)
    #for job in jobs2:
    #    print(job[0].samples)
    #print("End update")

@app.get("/requestLastLocalSample")
async def requestLastLocalSample(jobID : str):
    '''
        Return the last sample image associated with a job ID
    '''
    stmt = select(Job).where(Job.jobID == jobID)
    jobs = session.execute(stmt)
    
    # Dev warning: first() cancel the rest of the potential row in the result
    # and close the result. Can't call first() again on jobs after this
    firstJob = jobs.first()
    
    if firstJob is None:
        result = {
            "error" : "Job does not exist."
        }
        return JSONResponse(content=result)

    if firstJob[0].status == "PENDING":
        result = {
            "error" : "Job is not running yet."
        }
        return JSONResponse(content=result)

    if len(firstJob[0].samples) == 0:
        result = {
            "error" : "No samples received yet."
        }
        return JSONResponse(content=result)
    else:
        result = {
            "lastSample" : firstJob[0].samples[-1],
            "image" : firstJob[0].images[-1],
            "status" : firstJob[0].status
        }
        return JSONResponse(content=result)

    if jobID not in runningJobDB:
        result = {
            "error" : "Job not running."
        }
        return JSONResponse(content=result)
    
    entry = runningJobDB[jobID]
    if len(entry["samples"]) == 0:
        result = {
            "error" : "No samples received yet."
        }
        return JSONResponse(content=result)
    else:
        result = {
            "lastSample" : entry["samples"][-1],
            "image" : entry["images"][-1],
            "status" : entry["status"]
        }
        return JSONResponse(content=result)

async def parse_body(request: Request):
    data: bytes = await request.body()
    return data

@app.post("/updateRemoteJobExec")
async def updateRemoteJobExec(file: UploadFile, jobID: str = Form(...), lastSample: str = Form(...)):
    '''
        Upload a sample image and store it locally. The file is then 
        move to a local folder which is saved in the database.
    '''
    contents = await file.read()

    filename = file.filename
    
    if filename == '':
        result =  {'error': 'No file selected for uploading'}
        return JSONResponse(content=result)

    jobFolder = os.path.join(SAMPLE_FOLDER, jobID)
    if not os.path.isdir(jobFolder):
        os.mkdir( jobFolder )

    filePath = os.path.join(jobFolder, filename)
    with open( filePath, "wb") as f:
        f.write(contents)

    
    runningJobDB[jobID]["samples"].append(int(lastSample))
    runningJobDB[jobID]["images"].append(filePath)

if __name__ == '__main__':

    if not os.path.isdir(CONTROLLER_FOLDER):
        os.mkdir( CONTROLLER_FOLDER )

    if not os.path.isdir(SAMPLE_FOLDER):
        os.mkdir( SAMPLE_FOLDER )    

    uvicorn.run('localController:app', host='0.0.0.0', port=8000)

