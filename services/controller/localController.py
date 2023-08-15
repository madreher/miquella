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

runningJobDB = []

@app.post("/submit")
async def create_rendering_job(sceneID : int = 3, nSamples : int = 1000, freqOutput : int = 50):
    '''
        Send a query to perform a rendering task.
    '''
    jobId = str(uuid.uuid4())
    pendingJobDB.append({"jobID" : jobId, "sceneID" : sceneID, "nSamples" : nSamples, "freqOutput" : freqOutput})

    return Response(content=jobId, media_type="text/html")

@app.post("/requestJob")
async def provision_job():
    '''
        Send a rendering job to a server
    '''
    if len(pendingJobDB) > 0:
        runningJobDB.append(pendingJobDB[0])
        del pendingJobDB[0]

        return JSONResponse(content=runningJobDB[-1])
    else:
        return JSONResponse(content={})

if __name__ == '__main__':

    uvicorn.run('localController:app', host='0.0.0.0', port=8000)

