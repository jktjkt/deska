
def kinds():
	return ['vendor', 'service', 'interface_template', 'hardware', 'hardware_template', 'host', 'interface', 'host_template']


def atts(kind):
	return {'vendor': {}, 'service': {'note': 'text'}, 'hardware': {'warranty': 'date', 'purchase': 'date', 'vendor': 'identifier', 'cpu_num': 'int4', 'ram': 'int4', 'host': 'identifier', 'template_hardware': 'identifier', 'note_hardware': 'text'}, 'hardware_template': {'warranty': 'date', 'purchase': 'date', 'vendor': 'identifier', 'cpu_num': 'int4', 'ram': 'int4', 'template_hardware': 'identifier', 'note_hardware': 'text'}, 'host': {'hardware': 'identifier', 'note_host': 'text', 'template_host': 'identifier', 'service': 'identifier_set'}, 'interface_template': {'note': 'text', 'template_interface': 'identifier', 'mac': 'macaddr', 'ip6': 'ipv6', 'ip4': 'ipv4'}, 'host_template': {'note_host': 'text', 'template_host': 'identifier', 'service': 'identifier_set'}, 'interface': {'template_interface': 'identifier', 'mac': 'macaddr', 'note': 'text', 'host': 'identifier', 'ip6': 'ipv6', 'ip4': 'ipv4'}}[kind]


def embed():
	return {'interface': 'host'}


def template():
	return {'hardware': 'hardware_template', 'interface': 'interface_template', 'host': 'host_template', 'interface_template': 'interface_template', 'hardware_template': 'hardware_template', 'host_template': 'host_template'}


def merge():
	return {'hardware': 'host', 'host': 'hardware'}


def refs():
	return {'vendor': [], 'service': [], 'hardware': ['vendor'], 'hardware_template': ['vendor'], 'host': ['service'], 'interface_template': [], 'host_template': ['service'], 'interface': []}

