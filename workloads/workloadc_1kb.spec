# Workload C (1KB): 100% read (zipfian)
recordcount=104857600
operationcount=104857600
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=1024
field_len_dist=fixed
readallfields=true
writeallfields=true
splitlsm.enable_gc=false

readproportion=1
updateproportion=0
deleteproportion=0
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=zipfian
