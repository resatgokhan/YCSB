# Workload D (16KB): 95% read, 5% insert (latest)
recordcount=6553600
operationcount=6553600
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=16384
field_len_dist=fixed
readallfields=true
writeallfields=true
splitlsm.enable_gc=false

readproportion=0.95
updateproportion=0
deleteproportion=0
insertproportion=0.05
scanproportion=0
readmodifywriteproportion=0

requestdistribution=latest
