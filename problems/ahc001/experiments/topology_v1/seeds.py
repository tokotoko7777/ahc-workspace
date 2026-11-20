import hashlib

for i in range(8):
    raw = hashlib.sha256(f'ahc001-topology-v1-dev-{i}'.encode()).digest()
    print(int.from_bytes(raw[:8], 'big'))
