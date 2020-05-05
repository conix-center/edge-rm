import sqlite3

dbname = 'edgerm.db'

def refresh_db():
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("DROP TABLE IF EXISTS agents")
		c.execute("DROP TABLE IF EXISTS resources")
		c.execute("CREATE TABLE agents (id integer primary key autoincrement, conn text)")
		c.execute("CREATE TABLE resources (agentID integer, name text, value text)")
		conn.commit()

def add_agent(resources, connection):
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("INSERT INTO agents (conn) VALUES (?)", (connection,))
		agent_id = c.lastrowid
		for (rname, rval) in resources:
			c.execute("INSERT INTO resources VALUES (?,?,?)", (agent_id, rname, rval))
		conn.commit()
		return agent_id
	return None

def get_all():
	agents = {}
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("SELECT * FROM agents")
		rows = c.fetchall()
		for row in rows:
			agent_id = row[0]
			resources = []
			c.execute("SELECT * FROM resources WHERE agentID = ?",str(agent_id))
			rrows = c.fetchall()
			for rrow in rrows:
				rname = rrow[1]
				rval = rrow[2]
				resources.append((rname, rval))
			agents[agent_id] = resources
		conn.commit()
	return agents

def delete_agent(agent_id):
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("DELETE FROM agents WHERE id = ?", str(agent_id))
		c.execute("DELETE FROM resources WHERE agentID = ?", str(agent_id))
		conn.commit()
