FROM python:3.10-slim

WORKDIR /app

# Install system dependencies if needed (e.g. gcc for building native extensions)
RUN apt-get update && apt-get install -y --no-install-recommends \
    gcc \
    libc6-dev \
    network-manager \
    && rm -rf /var/lib/apt/lists/*

# Copy and install Python requirements
COPY requirements.txt .
RUN pip install --no-cache-dir -r requirements.txt

# Copy the rest of the application code
COPY . .

EXPOSE 8000

# Start FastAPI server using uvicorn
CMD ["uvicorn", "app.main:app", "--host", "0.0.0.0", "--port", "8000", "--reload"]
