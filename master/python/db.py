import sqlite3
import messages_pb2

dbname = 'edgerm.db'

def refresh_db():
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("DROP TABLE IF EXISTS agents")
		c.execute("DROP TABLE IF EXISTS resources")
		c.execute("DROP TABLE IF EXISTS attributes")
		c.execute("CREATE TABLE agents (id integer primary key autoincrement, conn text)")
		c.execute("CREATE TABLE resources (agentID integer, name text, type integer, scalar_value double, set_value text, text_value text)")
		c.execute("CREATE TABLE attributes (agentID integer, name text, type integer, scalar_value double, set_value text, text_value text)")
		conn.commit()

def add_agent(resources, attributes, connection):
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("INSERT INTO agents (conn) VALUES (?)", (connection,))
		agent_id = c.lastrowid
		for (rname, rtype, rval) in resources:
			if rtype == messages_pb2.Value.SCALAR:
				c.execute("INSERT INTO resources VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, rval, None, None))
			elif rtype == messages_pb2.Value.SET:
				for r in rval:
					c.execute("INSERT INTO resources VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, None, r, None))
			elif rtype == messages_pb2.Value.TEXT:
				c.execute("INSERT INTO resources VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, None, None, rval))
		for (rname, rtype, rval) in attributes:
			if rtype == messages_pb2.Value.SCALAR:
				c.execute("INSERT INTO attributes VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, rval, None, None))
			elif rtype == messages_pb2.Value.SET:
				for r in rval:
					c.execute("INSERT INTO attributes VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, None, r, None))
			elif rtype == messages_pb2.Value.TEXT:
				c.execute("INSERT INTO attributes VALUES (?,?,?,?,?,?)", (agent_id, rname, rtype, None, None, rval))
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
			attributes = []
			c.execute("SELECT * FROM resources WHERE agentID = ?",str(agent_id))
			rrows = c.fetchall()
			for rrow in rrows:
				rname = rrow[1]
				rtype = rrow[2]
				scalar_val = rrow[3]
				set_val = rrow[4]
				text_val = rrow[5]
				resources.append((rname, rtype, scalar_val, set_val, text_val))
			c.execute("SELECT * FROM attributes WHERE agentID = ?",str(agent_id))
			rrows = c.fetchall()
			for rrow in rrows:
				rname = rrow[1]
				rtype = rrow[2]
				scalar_val = rrow[3]
				set_val = rrow[4]
				text_val = rrow[5]
				attributes.append((rname, rtype, scalar_val, set_val, text_val))
			agents[agent_id] = (resources, attributes)
		conn.commit()
	return agents

def delete_agent(agent_id):
	with sqlite3.connect(dbname) as conn:
		c = conn.cursor()
		c.execute("DELETE FROM agents WHERE id = ?", str(agent_id))
		c.execute("DELETE FROM resources WHERE agentID = ?", str(agent_id))
		conn.commit()
