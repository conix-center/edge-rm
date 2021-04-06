FROM arm32v6/python:3.7-alpine3.12

COPY requirements.txt ./
RUN pip install --no-cache-dir -r requirements.txt

COPY hello.py ./

CMD [ "python", "./hello.py" ]