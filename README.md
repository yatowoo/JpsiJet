# $J/\psi$ Production in Jets

> From IDEA to PAPER.

Repository for references, analysis codes, useful scripts and related discussions.

Software: [AliPhysics](https://github.com/alisw/AliPhysics)@ROOT6

Main environment: AliEn World-wide Computing Grid

Local: R710.star.ustc.edu.cn `(Dual-E5520 + 16* Dual-E5504)`

> Development and post-processing are performed on local environment `(vAN-20190328_ROOT6)`. But for alien, all jobs would run on latest version after debug.

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

* EMCal & Jet:
[Intro.](https://indico.cern.ch/event/555035/contributions/2239719/attachments/1310149/1959997/EMCALframework.pdf "July 2016") |
[Doc.](http://alidoc.cern.ch/AliPhysics/master/READMEemcfw.html "Based on Doxygen") |
[Correction](https://indico.cern.ch/event/586577/contributions/2363131/attachments/1370126/2177061/EMCalCorrectionFramework_AnalysisTutorial_updated.pdf "Nov. 2016") |
[Embedding](https://indico.cern.ch/event/586577/contributions/2363130/attachments/1370183/2296645/rehlers.embedding.analysis.tutorial.nov.2016.v3.pdf "Nov. 2016") |

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

* ALICE-analysis-tutorial:
[Website](https://alice-analysis.web.cern.ch/content/tutorials) |
[Indico](https://indico.cern.ch/category/5819/ "Some events under category of ALICE Week") |
[Intro.](https://alice-doc.github.io/alice-analysis-tutorial/) |
[DataFlow](https://indico.cern.ch/event/752367/contributions/3116617/attachments/1704565/2858687/DPG_AnalysisTutorial_20181129.pdf "Starterkit - Nov.  2018") |
[LEGO train](https://indico.cern.ch/event/327364/contributions/1713450/attachments/635959/875472/juniorsDay-2.pdf "July 2014") |
[NanoAOD](https://indico.cern.ch/event/652886/contributions/2658039/attachments/1500603/2336871/nanoAODs.pdf "July 2017") |
[EventInfo](https://indico.cern.ch/event/327364/contributions/1713452/attachments/635962/875477/2014-07-04-kryshen-tutorial.pdf "July 2014") |
[Q-vector](https://indico.cern.ch/event/586577/contributions/2363129/attachments/1370104/2077614/flowvectors.pdf "Nov. 2016") |
[TrackInfo](https://indico.cern.ch/event/327364/contributions/1713452/attachments/635962/875475/TrackSelectionTutorial.pdf "July 2014") |
[PID](https://indico.cern.ch/event/555035/contributions/2236078/attachments/1307393/1959973/ParticleID_in_ALICE_15072016.pdf "July 2016") |
[Flow](https://indico.cern.ch/event/327364/contributions/1713456/attachments/635966/875483/flow_package_na.pdf "July 2014") |
[EventMixing](https://indico.cern.ch/event/652886/contributions/2666025/attachments/1500563/2337020/eventmixing_aohlson.pdf "July 2017") |
[Unfolding](https://indico.cern.ch/event/463952/contributions/1981524/attachments/1206439/1758559/unfolding.pdf "Dec. 2015") |
[Errors](https://indico.cern.ch/event/652886/contributions/2666026/attachments/1500620/2336893/20170726_stat_syst_unc.pdf "July 2017") |
[MC](jklein.web.cern.ch/jklein/mc/ "Dec. 2015"), [Generator](https://indico.cern.ch/event/327364/contributions/1713449/attachments/635957/875470/GeneratorsTutorials.pdf "July 2014") |
[RIVET](https://indico.cern.ch/event/555035/contributions/2236077/attachments/1307618/1959552/intro_rivet.pdf "MC tool - July 2016") |
[AnaControl](https://github.com/cbourjau/nittygriddy "ALICE+LHCb - Nov. 2017") |
[PlotStyle](https://indico.cern.ch/event/327364/contributions/1713455/attachments/635965/875481/main.pdf "July 2014") |
[Visual](https://indico.cern.ch/event/743750/contributions/3072931/attachments/1690231/2719440/MultiDimensionalVisualization_O2meeting22052018.pdf "July 2018") |
[QAtools](https://indico.cern.ch/event/743750/contributions/3072931/attachments/1690231/2719650/QAtools_Tutorial.pdf "July 2018") |
[Git](https://indico.cern.ch/event/616865/ "Git tips & GitHub usage - Mar. 2017") |
[alice-docs](https://indico.cern.ch/event/713442/contributions/2931242/attachments/1619989/2581360/The_new_ALICE_documentation_pages.pdf "With GitHub page - Mar. 2018") |
[Doxygen](https://indico.cern.ch/event/586577/contributions/2363128/attachments/1370076/2077505/documentYourCode.pdf "Documents for AliRoot & AliPhysics - Nov. 2016") |
[ML](https://indico.cern.ch/event/713442/contributions/2931240/attachments/1622111/2581475/2018-03-23_ML_tutorial.pdf "With SWAN - Mar. 2018") |
* ROOT & C++:
[Primer](https://root.cern/guides/primer) |
[Courses](https://root.cern/courses) |
[Example](https://root.cern.ch/doc/master/group__Tutorials.html "Tutorials inside ROOT") |
[PEP-ROOT6](https://pep-root6.github.io/docs/ "C++11 features, Parallellism, Vectorization, Declarative analysis, Python & debug") |
[ROOT5to6](https://alice-doc.github.io/alice-analysis-tutorial/analysis/ROOT5-to-6.html "Inside gitbook of Intro. - Mar. 2018") |
[Python](https://indico.cern.ch/event/652886/ "July 2017") |
[ModernC++](https://indico.cern.ch/event/752367/contributions/3116613/attachments/1704567/2861784/ModernCPP.pdf "Starterkit - Nov. 2018") |
[Pointers](https://indico.cern.ch/event/666222/contributions/2722821/attachments/1552323/2439274/Pointers_SAiola.pdf "ALICE+LHCb - Nov. 2017") |
[Profiling](https://indico.cern.ch/event/463952/contributions/1981522/attachments/1206438/1758208/alice15-tools-lecture.pdf "Dec. 2016") |
* Others:
[AliSW tutorial](http://alisw.github.io/) |
[AliBuild](http://alisw.github.io/alibuild/) |
[PCG](https://friederikebock.gitbooks.io/pcgtutorial/) |
[Debug@ALICE](https://dberzano.github.io/alice/debug/) |
[Starterkit2018](https://indico.cern.ch/event/752367 "ALICE + LHCb + SHiP - bash, python, git, alisw, ROOT & debug") |
* School & Course:
[INFN-ESC](https://agenda.infn.it/event/16941/ "https://agenda.infn.it/conferenceOtherViews.py?view=standard&confId=16941") |
[Huada-QCD](https://indico.ihep.ac.cn/event/7841/ "The 7th Huada School on QCD - Jet") |
[Geant4-events](http://geant4.web.cern.ch/past-events "Geant4 workshop, course and meeting") |
[GSI-Summer](https://theory.gsi.de/stud-pro/ "HGS-HIRe International Summer Student Program at GSI, for Europe or GSI/FAIR partner countries") |
[France-China](https://cn.ambafrance.org/-Les-Ecoles-d-ete-France-Excellence- "The France Excellence Summer Schools Program - Physics of the two infinities") |
[CERN-Summer](https://careers.cern/summer "Summer Student Programme & Openlab summer Student Programme") |
[CERN-Fermi](https://indico.cern.ch/event/795313/overview "Joint CERN-Fermilab Hadron Collider Physics Summer School - at young postdocs and senior PhD students") |
[Euro-Inst.](http://www.esi-archamps.eu/Thematic-Schools/Discover-ESIPAP "European School of Instrumentation in Particle & Astroparticle Physics") |
[Trigger-DAQ](https://indico.cern.ch/event/739424/ "ISOTDAQ 2019 - International School of Trigger and Data AcQuisition") |

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

To understood the $J/\psi$ production mechanism, and explain the lack of observed polarization in high $p_{T}$ range. Previous results from LHCb and CMS are disagree with NRQCD predictions implemented by Pythia8.

![Result_LHCb](Media/Result_LHCb.png)
![Result_CMS](Media/Result_CMS.png)

### Reference

1. Baumgart, Matthew, et al. "Probing quarkonium production mechanisms with jet substructure." JHEP 1411 (2014) 003 [[INSPIRE](http://inspirehep.net/record/1299682)]
2. LHCb Collaboration, "Study of $J/\psi$ Production in Jets", Phys.Rev.Lett. 118 (2017) no.19, 192001 [[INSPIRE](http://inspirehep.net/record/1509507)]
3. Kang, Zhong-Bo, et al. "$J/\psi$ production and polarization within a jet." Phys.Rev.Lett. 119 (2017) no.3, 032001. [[INSPIRE](http://inspirehep.net/record/1512930/)]
4. Bain, Reggie, et al. "NRQCD Confronts LHCb Data on Quarkonium Production within Jets." Phys.Rev.Lett. 119 (2017) no.3, 032002. [[INSPIRE](http://inspirehep.net/record/1514263/)]
5. CMS Collaboration, "Production of prompt and nonprompt $J/\psi$ mesons in jets in pp collisions at $\sqrt{s}=5.02~TeV$", CMS-PAS-HIN-18-012, [[CDS](http://cds.cern.ch/record/2318344)]
6. Dai, Lin, et al. "Quarkonium Polarization and the Long Distance Matrix Elements Hierarchies using Jet Substructure" Phys.Rev. D96 (2017) no.3, 036020 [[INSPIRE](http://inspirehep.net/record/1613333)]
7. Dai, Lin, et al. "Heavy quark jet fragmentation." JHEP 1809 (2018) 109 [[INSPIRE](http://inspirehep.net/record/1673397)]
8. Dai, Lin. "Applications of QCD Effective Theories to the Physics of Jets and Quarkonium Production." PhD Diss. University of Pittsburgh, 2018. [[PDF](http://d-scholarship.pitt.edu/34888/1/dissertation-lin-dai-rev1.pdf)]

## Physical Object

The jet fragmentation function $z(J/\psi)$, which is the $p_{T}$ fraction carried by $J/\psi$ in jets.

In proton-proton collision at $\sqrt{s}=13~TeV$, $J/\psi$ mesons are reconstructed using their dielectron channel in **_central barrel_**. The dielectron events were selected offline with the L0 trigger system, requiring **_one_** leg in EMCal/DCal and exceed the L1 energy threshold. Other analysis cuts deployed for electron identification will be presented in following sections. Prompt and non-prompt $J/\psi$ mesons are seperated by pesudo-proper decay length.

Jets are clustered using the anti-$k_{T}$ algorithm from charged tracks, with R=0.2, which provides by [fastjet](http://fastjet.fr "v3.2.1") package.

## Datasets

## QA

## Signal Extraction

### PID for electron

* Basic strategy: Using dE/dx (from TPC) and E/p (from EMCal) to indentify electrons, and only TPC signal to exclude kaons and protons.
* Hybrid method: To add TOF or/and TRD as complements.

### $J/\psi$ reconstruction

1. Invariant mass spectrum. Fit with Crystal-Ball (signal) + pol2 (background) function.
2. Combination of EMCal L1 trigger classes (EG1/EG2/DG1/DG2).
3. Cross section vs $p_{T}$, after normalized with EMCal rejection factor.
4. Pseudo-proper decay length $\ell_{J/\psi}=L_{xyz}m_{J/\psi}c/|p_{ee}|$, for the seperation of prompt and non-prompt $J/\psi$. $L_{xyz}$ is the distance between the primary and dielectron vertices. The prompt, non-prompt and background components are parameterized using data and MC events after unfolded with [$_{s}\mathcal{P}lot$](http://inspirehep.net/record/644725) technique.

### Nano AODs

### Jet finder

## Correction

### EMCal correction and embedding framework

> Reference:
> [alidoc](http://alidoc.cern.ch/AliPhysics/master/READMEemcCorrections.html) |
> [PCG-tutorial](https://friederikebock.gitbooks.io/pcgtutorial/content/AliPhysicsAndGrid/corrframework.html) |
> ALICE-analysis-tutorial ([Correction](https://indico.cern.ch/event/586577/contributions/2363131/attachments/1370126/2177061/EMCalCorrectionFramework_AnalysisTutorial_updated.pdf "Nov. 2016"), [Embedding](https://indico.cern.ch/event/586577/contributions/2363130/attachments/1370183/2296645/rehlers.embedding.analysis.tutorial.nov.2016.v3.pdf "Nov. 2016")) |

* Bad channel map, energy calibration, time calibration, cell-level crosstalk.
* Cluster exotics, enery non-linearity, track matching, hadronic correction and PHOS tender.

### $J/\psi$ acceptance and efficiency

* Detector geometrical coverage and acceptance.
* Trigger, tracking reconstruction, electron identification and selection efficiency of $e^{+}e^{-}$ pairs.

### Unfolding of jet $p_{T}$ resolution

The procedure is carried out in two dimensions, z and jet $p_{T}$ with D'Agostini's iterative algorithm, which is avalaible in [RooUnfold](http://inspirehep.net/record/898599) package. In this step, we should produce a four-dimensional detector-response (MC & data) matrix for prompt and non-prompt $J/\psi$.

## Systematic uncertainties

## Preliminary Result

## Publication