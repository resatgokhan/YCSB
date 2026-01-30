# Workload A (16KB): 50% read, 50% update (zipfian)
recordcount=6553600
operationcount=6553600
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=16384
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
