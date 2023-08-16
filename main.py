from fastapi import FastAPI, Request
import uvicorn
import requests
import json

app = FastAPI()
state = 'valid'
 
@app.get("/")
def read_root():
    return {"state": "Working"}


@app.post("/state/get")
async def get_state(request: Request):
    global state

    device_data = await request.json()
    print(f"POSTED DATA: {device_data}")

    return {"state": state}

@app.post("/state/set")
async def set_state(request: Request):
    global state

    new_state = await request.json()
    new_state = new_state.get('new_state')
    print(f"POST DATA: {new_state}")

    state = new_state
    return {"state": state}



@app.get("/api/{token}/{status}")
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
    config = uvicorn.Config("main:app", port=5200, log_level="info", reload=True, host='0.0.0.0')
    server = uvicorn.Server(config)
    server.run()

