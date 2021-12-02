# Prime+Scope

This repository is the open source component of our [Prime+Scope paper](https://www.esat.kuleuven.be/cosic/publications/article-3405.pdf) (CCS '21).

It comprises the following artifacts:
- `primescope_demo`: a quick and simple demonstration of Prime+Scope
- `evsets`: eviction set construction (PS-style) for inclusive and non-inclusive Intel caches
- `primetime`: a tool to find fast and effective PRIME access patterns
- `covert_inclusive`: a cross-core covert channel that encodes information through variable-time accesses

Configuration, build and execution instructions are outlined in the relevant folders. 

## How to cite this work

```
@inproceedings{PrimeScope,
  author    = {Purnal, Antoon and Turan, Furkan and Verbauwhede, Ingrid},
  title     = {Prime+Scope: Overcoming the Observer Effect for High-Precision Cache Contention Attacks},
  booktitle = {ACM SIGSAC Conference on Computer and Communications Security (CCS)},
  year      = {2021},
}
```