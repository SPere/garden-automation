using GardenApi.Data;
using MySqlConnector;
using Microsoft.EntityFrameworkCore;

var builder = WebApplication.CreateBuilder(args);

builder.Services.AddControllers();

var connectionString = builder.Configuration.GetConnectionString("DefaultConnection")!;
// Inject password from MYSQL_ROOT_PASSWORD — matches K8s secret mysql-secrets/root-password
var dbPassword = Environment.GetEnvironmentVariable("MYSQL_ROOT_PASSWORD");
if (!string.IsNullOrEmpty(dbPassword))
{
    var csb = new MySqlConnectionStringBuilder(connectionString) { Password = dbPassword };
    connectionString = csb.ToString();
}

builder.Services.AddDbContext<AppDbContext>(options =>
    options.UseMySql(connectionString, new MySqlServerVersion(new Version(8, 0, 0))));

var app = builder.Build();

using (var scope = app.Services.CreateScope())
{
    try
    {
        var db = scope.ServiceProvider.GetRequiredService<AppDbContext>();
        await db.Database.EnsureCreatedAsync();
    }
    catch (Exception ex)
    {
        var logger = scope.ServiceProvider.GetRequiredService<ILogger<Program>>();
        logger.LogError(ex, "Database initialization failed — app will continue without DB");
    }
}

app.MapControllers();
app.Run();
