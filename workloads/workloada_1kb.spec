# Workload A (1KB): 50% read, 50% update (zipfian)
recordcount=104857600
operationcount=104857600
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=1024
field_len_dist=fixed
readallfields=true
writeallfields=true
splitlsm.enable_gc=false

readproportion=0.5
updateproportion=0.5
deleteproportion=0
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=zipfian
