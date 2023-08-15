#!/usr/bin/env python3

from fastapi import FastAPI, Form, Request, status, Query
from fastapi.responses import JSONResponse, HTMLResponse, FileResponse, RedirectResponse, Response
from fastapi.staticfiles import StaticFiles
from fastapi.templating import Jinja2Templates
from fastapi import FastAPI, File, UploadFile
from typing import Union
import uvicorn
import os
import time
import shutil
import uuid
import sys
import argparse

app = FastAPI()
app.mount("/static", StaticFiles(directory="static"), name="static")
templates = Jinja2Templates(directory="templates")

pendingJobDB = []

runningJobDB = {}

@app.post("/submit")
async def create_rendering_job(sceneID : int = 3, nSamples : int = 1000, freqOutput : int = 50):
    '''
        Send a query to perform a rendering task.
    '''
    jobId = str(uuid.uuid4())
    pendingJobDB.append(
        {
            "jobID" : jobId, 
            "sceneID" : sceneID, 
            "nSamples" : nSamples, 
            "freqOutput" : freqOutput,
            "samples" : [],
            "images" : []
        })

    return Response(content=jobId, media_type="text/html")

@app.post("/requestJob")
async def provision_job():
    '''
        Send a rendering job to a server
    '''
    if len(pendingJobDB) > 0:
        jobID = pendingJobDB[0]["jobID"]
        runningJobDB[jobID] = pendingJobDB[0]
        del pendingJobDB[0]

        return JSONResponse(content=runningJobDB[jobID])
    else:
        return JSONResponse(content={})

@app.post("/updateLocalJobExec")
async def updateLocalJobExec(jobID : str, filePath : str, lastSample : int):
    '''
        Update the runningJobDB with the last output done by a worker.
    '''
    if jobID not in runningJobDB:
        raise RuntimeError("Received an unknown JobID.")
    
    runningJobDB[jobID]["samples"].append(lastSample)
    runningJobDB[jobID]["images"].append(filePath)

@app.get("/requestLastLocalSample")
async def requestLastLocalSample(jobID : str):
    '''
        Return the last sample image associated with a job ID
    '''

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
            "image" : entry["images"][-1]
        }
        return JSONResponse(content=result)

if __name__ == '__main__':

    uvicorn.run('localController:app', host='0.0.0.0', port=8000)

