from fastapi import FastAPI, Request
import uvicorn
import requests
import json
import random
import string

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

@app.post("/api/get_user/{id_number}")
async def set_state(request: Request, id_number: str):
    
    # response = requests.request("GET", "https://randomuser.me/api/", headers={}, data={}).json()
    
    first_name = random.choices(string.ascii_uppercase, k=random.randint(3, 15))
    middle_name = random.choices(string.ascii_uppercase, k=random.randint(3, 12))
    last_name = random.choices(string.ascii_uppercase, k=random.randint(3, 10))
    dob = f"{random.randint(1930, 2022)}-{random.randint(1, 12)}-{random.randint(1, 30)}"
    area_of_birth = random.choices(string.ascii_uppercase, k=random.randint(10, 15))
    sex = random.choice(['M', 'F'])
    mother_name = random.choices(string.ascii_uppercase, k=random.randint(3, 15))
    
    response_string = f"{id_number}:NULL:{first_name}:{middle_name}:{last_name}:{dob}:{area_of_birth}:{sex}:{mother_name}:200:Successful"
    
    return response_string



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

