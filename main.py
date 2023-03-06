from fastapi import FastAPI
import uvicorn
import requests
import json

app = FastAPI()


@app.get("/")
def read_root():
    return {"Status": "Working"}


@app.post("/api/{token}/{status}")
def create_item(token: str, status: str):
    
    url = "https://aws-demo.razorinformatics.co.ke/api/v1/sensors"
    payload = json.dumps({"status": status})
    headers = {
    'Accept': 'application/json',
    'Authorization': f'Bearer {token}',
    'Content-Type': 'application/json'
    }
    
    response = requests.request("POST", url, headers=headers, data=payload)
    
    return response.json()


if __name__ == "__main__":
    config = uvicorn.Config("main:app", port=5200, log_level="info", reload=True)
    server = uvicorn.Server(config)
    server.run()

