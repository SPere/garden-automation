# Garden WebAPI — Implementation Plan

## Location
`src/webapi/` (ASP.NET Core 8 Web API, project name `GardenApi`)

## Tech Stack
- .NET 8 Web API (controller-based)
- EF Core + Pomelo.EntityFrameworkCore.MySql
- Filesystem for image storage

## Project Structure
```
src/webapi/
├── GardenApi.csproj
├── Program.cs
├── appsettings.json                  # production (K8s) defaults
├── appsettings.Development.json      # local dev overrides
├── Controllers/
│   ├── HealthController.cs
│   ├── ImageController.cs
│   └── ReadingController.cs
├── Data/
│   └── AppDbContext.cs
└── Models/
    └── SensorReading.cs
```

## Endpoints

### GET /healthz
Returns `200 OK` with `{"status":"ok"}`

### POST /save-image  (multipart/form-data)
Parts:
- `metadata` — JSON string e.g. `{"source":"abcdefghij"}`
- `image` — image file

Save path: `{IMAGE_STORAGE_PATH}/{yyMMdd}/{HHmmss}_{source}{ext}`
e.g. `./images/260916/143022_abcdefghij.jpg`

Config key: `ImageStorage:Path` (appsettings) or `IMAGE_STORAGE_PATH` (env var, takes priority)

Returns `200` with `{"path":"<saved path>"}`

### POST /save-reading  (application/json)
Body:
```json
{"temp":25.1,"hum":71.0,"soil1":1152,"soil2":1104,"soil3":1708,"soil4":1535,"light":3504,"source":"1A2B3C4D5E6F"}
```
All numeric fields stored as `double`. `source` is a string.
Saves async to MySQL `SensorReadings` table. Returns `202 Accepted`.

## EF Model: SensorReading
| Field     | Type     | Notes              |
|-----------|----------|--------------------|
| Id        | int      | PK, auto-increment |
| Temp      | double   |                    |
| Hum       | double   |                    |
| Soil1–4   | double   |                    |
| Light     | double   |                    |
| Source    | string   |                    |
| CreatedAt | DateTime | set at save time   |

Table auto-created via `EnsureCreatedAsync()` at startup.

## MySQL Connection (from existing K8s deployment)
- Host:     `mysql.mysql.svc.cluster.local`
- Port:     `3306`
- Database: `mydb`
- User:     `root`
- Password: injected via `MYSQL_ROOT_PASSWORD` env var  
             (from K8s secret `mysql-secrets`, key `root-password`)

The app reads `MYSQL_ROOT_PASSWORD` at startup and injects it into the connection string.
In local dev, set the password directly in `appsettings.Development.json`.

## Configuration (appsettings.json — production defaults)
```json
{
  "ConnectionStrings": {
    "DefaultConnection": "Server=mysql.mysql.svc.cluster.local;Port=3306;Database=mydb;User=root;"
  },
  "ImageStorage": {
    "Path": "/images"
  }
}
```

## K8s Deployment Notes
- Set `MYSQL_ROOT_PASSWORD` from secret `mysql-secrets` key `root-password`
- Set `IMAGE_STORAGE_PATH` to the mounted volume path
- Mount a PVC at the image storage path for persistence
