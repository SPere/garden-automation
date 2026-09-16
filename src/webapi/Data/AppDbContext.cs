using GardenApi.Models;
using Microsoft.EntityFrameworkCore;

namespace GardenApi.Data;

public class AppDbContext(DbContextOptions<AppDbContext> options) : DbContext(options)
{
    public DbSet<SensorReading> SensorReadings => Set<SensorReading>();
}
