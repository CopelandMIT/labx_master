import sqlite3
import os

# Define the path for the SQLite database file
db_path = "/home/daniel/labx_master/central_server_code/data/database/lab_in_a_box.db"

# Connect to the SQLite database (creates it if it doesn't exist)
conn = sqlite3.connect(db_path)
cursor = conn.cursor()

# SQL statements to create tables based on the provided schema
create_tables_sql = [
    """
    CREATE TABLE IF NOT EXISTS participants (
        participant_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        age INTEGER,
        gender TEXT,
        other_details TEXT
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS activities (
        activity_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        description TEXT
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS data_captures (
        capture_id INTEGER PRIMARY KEY AUTOINCREMENT,
        participant_id INTEGER,
        start_time TIMESTAMP,
        end_time TIMESTAMP,
        activity_id INTEGER,
        room_id INTEGER,
        description TEXT,
        nlp_server TEXT,
        FOREIGN KEY (participant_id) REFERENCES participants(participant_id),
        FOREIGN KEY (activity_id) REFERENCES activities(activity_id),
        FOREIGN KEY (room_id) REFERENCES rooms(room_id)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS alerts (
        alert_id INTEGER PRIMARY KEY AUTOINCREMENT,
        deployed_sensor_id INTEGER,
        capture_id INTEGER,
        timestamp TIMESTAMP,
        alert_type TEXT,
        description TEXT,
        resolved BOOLEAN,
        FOREIGN KEY (deployed_sensor_id) REFERENCES deployed_sensors(deployed_sensor_id),
        FOREIGN KEY (capture_id) REFERENCES data_captures(capture_id)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS files (
        file_id INTEGER PRIMARY KEY AUTOINCREMENT,
        filename TEXT NOT NULL,
        filetype TEXT,
        filesize INTEGER,
        duration FLOAT,
        created_at TIMESTAMP,
        capture_id INTEGER,
        deployed_sensor_id INTEGER,
        FOREIGN KEY (capture_id) REFERENCES data_captures(capture_id),
        FOREIGN KEY (deployed_sensor_id) REFERENCES deployed_sensors(deployed_sensor_id)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS rooms (
        room_id INTEGER PRIMARY KEY AUTOINCREMENT,
        name TEXT NOT NULL,
        location TEXT,
        description TEXT
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS sensors (
        sensor_id INTEGER PRIMARY KEY AUTOINCREMENT,
        sensor_type_id INTEGER,
        serial_number TEXT,
        FOREIGN KEY (sensor_type_id) REFERENCES sensor_types(sensor_type_id)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS sensor_types (
        sensor_type_id INTEGER PRIMARY KEY AUTOINCREMENT,
        type_code TEXT NOT NULL,
        model TEXT,
        manufacturer TEXT
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS deployed_sensors (
        deployed_sensor_id INTEGER PRIMARY KEY AUTOINCREMENT,
        sensor_id INTEGER,
        room_id INTEGER,
        sensor_location_id INTEGER,
        connected BOOLEAN,
        FOREIGN KEY (sensor_id) REFERENCES sensors(sensor_id),
        FOREIGN KEY (room_id) REFERENCES rooms(room_id),
        FOREIGN KEY (sensor_location_id) REFERENCES sensor_locations(sensor_location_id)
    )
    """,
    """
    CREATE TABLE IF NOT EXISTS sensor_locations (
        sensor_location_id INTEGER PRIMARY KEY AUTOINCREMENT,
        room_id INTEGER,
        description TEXT,
        x_position FLOAT,
        y_position FLOAT,
        z_position FLOAT,
        roll FLOAT,
        pitch FLOAT,
        yaw FLOAT,
        FOREIGN KEY (room_id) REFERENCES rooms(room_id)
    )
    """
]

# Execute each SQL statement to create the tables
for create_table_sql in create_tables_sql:
    cursor.execute(create_table_sql)

# Commit the changes and close the connection
conn.commit()
conn.close()

print(f"Database created at {db_path}")
