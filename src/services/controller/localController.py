#!/usr/bin/env python3

from fastapi import FastAPI, Form, Request
from fastapi.responses import JSONResponse,Response, FileResponse
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi import FastAPI, File, UploadFile

from jobTable import JobDatabase

import uvicorn
import os

################################ DATABASE ###################################
database = JobDatabase(databasePath="sqlite://", echo=False, createDatabase=True)
################################ DATABASE ###################################

################################ WEBSERVER ###################################
app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")
################################ WEBSERVER ###################################

CONTROLLER_FOLDER = os.path.join(os.environ["HOME"], ".miquella")
SAMPLE_FOLDER = os.path.join(CONTROLLER_FOLDER, "samples")




@app.post("/submitJob")
async def create_rendering_job(sceneID : int = 3, nSamples : int = 1000, freqOutput : int = 50):
    '''
        Send a query to perform a rendering task.
    '''
    
    # Add the job to the databse
    jobID = database.addJob(sceneID=sceneID, nSamples=nSamples, freqOutput=freqOutput)

    return Response(content=jobID, media_type="text/html")

@app.post("/requestJob")
async def provision_job():
    '''
        Send a rendering job to a server
    '''

    result = database.getRenderingJobForServer()
    return JSONResponse(content=result)


@app.post("/updateLocalJobExec")
async def updateLocalJobExec(jobID : str, filePath : str, lastSample : int):
    '''
        Update the runningJobDB with the last output done by a worker.
    '''

    result = database.addSampleToJob(jobID=jobID, filePath=filePath, lastSample=lastSample)
    return JSONResponse(content=result)

@app.post("/cancelJob")
async def cancelJob(jobID : str):
    '''
    Cancel a job which is either in pending or running state.
    '''

    result = database.cancelJob(jobID=jobID)
    return JSONResponse(content=result)


@app.post("/removeJob")
async def removeJob(jobID : str):
    '''
    Remove a job from the database regardless of its current status.
    '''
    result = database.removeJob(jobID=jobID)
    return JSONResponse(content=result)

@app.get("/requestLastLocalSample")
async def requestLastLocalSample(jobID : str):
    '''
        Return the last sample image associated with a job ID. The image is provided as a file path
        and assumes that the client has access to the same filesystem as the client.
    '''
    result = database.getLastSampleFromJob(jobID=jobID)
    return JSONResponse(content=result)


@app.get("/requestLastRemoteSample")
async def requestLastRemoteSample(jobID : str):
    '''
    Return the last sample image associated with a job ID. The image is provided for download 
    and is meant for cases where the controller and the client are not on the same filesystem.
    '''
    result = database.getLastSampleFromJob(jobID=jobID)


    # Convert the result to str 
    if "lastSample" in result:
        result["lastSample"] = str(result["lastSample"])
    if "image" in result and result["image"] != "":
        return FileResponse(path=result["image"], headers=result)
    else:
        return JSONResponse(content = {}, headers=result)

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
        f.close()

    result = database.addSampleToJob(jobID=jobID, filePath=filePath, lastSample=int(lastSample))
    return result

@app.get("/requestListAllJobs")
async def requestListAllJobs():
    '''
        Return all the jobs in a json format. 
    '''
    result = database.getCompleteListOfJobs()
    return JSONResponse(content=result)

if __name__ == '__main__':

    if not os.path.isdir(CONTROLLER_FOLDER):
        os.mkdir( CONTROLLER_FOLDER )

    if not os.path.isdir(SAMPLE_FOLDER):
        os.mkdir( SAMPLE_FOLDER )    

    uvicorn.run('localController:app', host='0.0.0.0', port=8000)

