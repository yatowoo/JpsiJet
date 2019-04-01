# J/$\psi$ Production in Jets

> From IDEA to PAPER.

Repository for references, analysis codes, useful scripts and related discussions.

Software: [AliPhysics](https://github.com/alisw/AliPhysics)@ROOT6

Main environment: AliEn World-wide Computing Grid

Local: R710.star.ustc.edu.cn (Dual-E5520 + 16* Dual-E5504)

> Development and post-processing are performed on local environment (vAN-20190328_ROOT6). But for alien, all jobs would run on latest version after debug.

Author: [Yìtāo WÚ](mailto:yitao.wu@cern.ch)

## Resource

Software:
[AliSW@GitHub](https://github.com/alisw) |
[GitLab](http://gitlab.cern.ch/) |
[AliDoc](http://alidoc.cern.ch/) |
Offline Database ([OADB](https://indico.cern.ch/event/128634/contributions/112886/attachments/86273/123628/11-03-09-OW-OADB.pdf "Offline Analysis Database - /eos/experiment/alice/analysis-data/OADB/"), [OCDB](http://alice-offline.web.cern.ch/Activities/ConditionDB.html "Offline Constant Database - alien:///alice/data/[year]/OCDB/[Detector]/*/Data/Run*.root")) |
[ROOT](https://root.cern/) |
[HEPforge](https://hepforge.org/) |

Documents:
[Twiki](https://twiki.cern.ch/twiki/bin/viewauth/ALICE) |
[Public & Analysis Note](https://alice-notes.web.cern.ch) |
[ALICE Offline](http://alice-offline.web.cern.ch/) |
[ALICE Official](http://alice.cern.ch/) |

Service:
[AliEn](http://alien.web.cern.ch/) |
[AliMonitor](http://alimonitor.cern.ch) |
[Glance](https://glance.cern.ch/alice/membership/ "ALICE Member - Insititute, People & Service") |
[JIRA](https://alice.its.cern.ch/ "Issue Tracking System") |
[Vidyo](http://information-technology.web.cern.ch/services/fe/vidyo "Client login URL: https://vidyoportal.cern.ch") |
[CERN Account](https://account.cern.ch/account/) |
[CERN Service](https://resources.web.cern.ch/resources/Manage/ListServices.aspx "Resource Portal - list all available services") |
[CA](https://ca.cern.ch/ca/ "CERN Certificate Authority - Grid, EduRoam, Code signing") |
[CERN Mail](https://mmmservices.web.cern.ch/mmmservices/) |
[e-groups](https://e-groups.cern.ch/ "CERN mail lists") |
[CERNBox](https://cernbox.cern.ch "The cloud storage solution from CERN") |
[SWAN](http://swan.cern.ch/ "Provide virtual environment by cloud storage service - CERNBox") |

Tutorial:
[AliSW tutorial](http://alisw.github.io/) |
[AliBuild](http://alisw.github.io/alibuild/) |
[ALICE-analysis-tutorial](https://alice-doc.github.io/alice-analysis-tutorial/) |
[PCG](https://friederikebock.gitbooks.io/pcgtutorial/) |
[Debug@ALICE](https://dberzano.github.io/alice/debug/) |

Conference & Meeting:
[ALICE Conference](http://alice-conferences.web.cern.ch/) |

* Indico@CERN:
[ALICE Week](https://indico.cern.ch/category/6871/) |
[PWG](https://indico.cern.ch/category/302/) |
[EMCal](https://indico.cern.ch/category/874/) |
[ITS](https://indico.cern.ch/category/6759/) |
[TPC](https://indico.cern.ch/category/14/) |
* Other Indico:
[Indico@IHEP](https://indico.ihep.ac.cn/category/208/) |
[Indico@USTC-PNP](http://pnp.ustc.edu.cn/indico) |
[Indico@IN2P3](https://indico.in2p3.fr/category/100/) |
* Quark Matter:
[QM2019](http://qm2019.ccnu.edu.cn/) |
[QM2018](http://qm2018.infn.it/) |
[QM2017](http://qm2017.phy.uic.edu/) |
[QM2015](http://qm2015.riken.jp/) |
[QM2014](https://indico.cern.ch/event/219436/) |
[QM2012](https://indico.cern.ch/event/181055/) |
* Hard Probe:
[HP2018](https://indico.cern.ch/event/634426/) |
[HP2016](http://hp2016.ccnu.edu.cn/) |
[HP2015](http://www.physics.mcgill.ca/hp2015/) |
* Strangeness Quark Matter:
[SQM2019](https://sqm2019.ba.infn.it/) |
[SQM2017](https://indico.cern.ch/event/576735/) |
* LHC Physics:
[LHCP2019](http://lhcp2019.buap.mx/) |
[LHCP2018](http://lhcp2018.bo.infn.it/) |
[LHCP2017](http://lhcp2017.physics.sjtu.edu.cn/) |
[LHCP2016](http://lhcp2016.hep.lu.se/) |
[LHCP2015](http://hepd.pnpi.spb.ru/lhcp2015/index.php/) |
[LHCP2014](https://indico.cern.ch/event/279518/) |
[LHCP2013](https://indico.cern.ch/event/210555/) |
* Others:
[VCI](https://vci.hephy.at/home/ "Vienna Conference on Instrumentation")

Publication:
[InspierHEP](http://inspirehep.net) |
[CERN Document Server](http://cds.cern.ch/) |
[HEPData](https://www.hepdata.net/) |
[Google Scholar](http://scholar.google.com) |
[Microsoft Academic](https://academic.microsoft.com/) |
[SCImago Journal Rank](https://www.scimagojr.com/journalrank.php?category=3106) |

## Background & Motivation

To understood the J/$\psi$ production mechanism, and explain the lack of observed polarization in high $p_{T}$ range. Previous results from LHCb and CMS are disagree with NRQCD predictions implemented by Pythia8.

### Reference

1. Baumgart, Matthew, et al. "Probing quarkonium production mechanisms with jet substructure." Journal of High Energy Physics 2014.11 (2014): 3. [[INSPIRE](http://inspirehep.net/record/1299682)]
2. LHCb Collaboration, "Study of J/$\psi$ Production in Jets", Phys.Rev.Lett. 118 (2017) no.19, 192001 [[INSPIRE](http://inspirehep.net/record/1509507)]
3. Kang, Zhong-Bo, et al. "J/$\psi$ production and polarization within a jet." Phys.Rev.Lett. 119 (2017) no.3, 032001. [[INSPIRE](http://inspirehep.net/record/1512930/)]
4. Bain, Reggie, et al. "NRQCD Confronts LHCb Data on Quarkonium Production within Jets." Phys.Rev.Lett. 119 (2017) no.3, 032002. [[INSPIRE](http://inspirehep.net/record/1514263/)]
5. CMS Collaboration, "Production of prompt and nonprompt J/$\psi$ mesons in jets in pp collisions at $\sqrt{s}=5.02~TeV$", CMS-PAS-HIN-18-012, [[CDS](http://cds.cern.ch/record/2318344)]
6. Dai, Lin, et al. "Quarkonium Polarization and the Long Distance Matrix Elements Hierarchies using Jet Substructure" Phys.Rev. D96 (2017) no.3, 036020 [[INSPIRE](http://inspirehep.net/record/1613333)]
7. Dai, Lin, et al. "Heavy quark jet fragmentation." JHEP 1809 (2018) 109 [[INSPIRE](http://inspirehep.net/record/1673397)]
8. Dai, Lin. "Applications of QCD Effective Theories to the Physics of Jets and Quarkonium Production." PhD Diss. University of Pittsburgh, 2018. [[PDF](http://d-scholarship.pitt.edu/34888/1/dissertation-lin-dai-rev1.pdf)]

## Physical Object

The jet fragmentation function $z(J/\psi)$, which is the $p_{T}$ fraction carried by J/$\psi$ in jets.

## Datasets

## QA

## Signal Extraction

## Correction

## Systematics

## Preliminary Result

## Publication