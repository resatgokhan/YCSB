# Mixed workload: 70% read, 20% update, 10% delete
recordcount=100000
operationcount=100000
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=100
field_len_dist=constant
readallfields=true
writeallfields=true

readproportion=0.7
updateproportion=0.2
deleteproportion=0.1
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=zipfian
