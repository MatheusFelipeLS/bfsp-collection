See discussions, stats, and author profiles for this publication at: https://www.researchgate.net/publication/282999920 

## New block properties for flowshop scheduling with blocking and their application in an iterated greedy algorithm 

**Article** _in_ International Journal of Production Research · September 2015 

DOI: 10.1080/00207543.2015.1076941 

CITATIONS 

32 

READS 

791 

**6 authors** , including: 





Jian-Ya Ding Tsinghua University **16** PUBLICATIONS **842** CITATIONS SEE PROFILE Jatinder N. D. Gupta University of Alabama in Huntsville **470** PUBLICATIONS **14,479** CITATIONS SEE PROFILE 





Shiji Song Tsinghua University **351** PUBLICATIONS **13,838** CITATIONS SEE PROFILE Rui Zhang Xiamen University of Technology **73** PUBLICATIONS **2,181** CITATIONS SEE PROFILE 

All content following this page was uploaded by Jian-Ya Ding on 28 January 2016. 

The user has requested enhancement of the downloaded file. 

**International Journal of Production Research** 



**ISSN: 0020-7543 (Print) 1366-588X (Online) Journal homepage: http://www.tandfonline.com/loi/tprs20** 



# **New block properties for flowshop scheduling with blocking and their application in an iterated greedy algorithm** 

### **Jian-Ya Ding, Shiji Song, Jatinder N.D. Gupta, Cheng Wang, Rui Zhang & Cheng Wu** 

**To cite this article:** Jian-Ya Ding, Shiji Song, Jatinder N.D. Gupta, Cheng Wang, Rui Zhang & Cheng Wu (2015): New block properties for flowshop scheduling with blocking and their application in an iterated greedy algorithm, International Journal of Production Research, DOI: <u>10.1080/00207543.2015.1076941</u> 

**To link to this article:** <u>http://dx.doi.org/10.1080/00207543.2015.1076941</u> 











Published online: 11 Sep 2015. Submit your article to this journal 



Article views: 42 



View related articles 



View Crossmark data 

Full Terms & Conditions of access and use can be found at http://www.tandfonline.com/action/journalInformation?journalCode=tprs20 

**Download by:** [Tsinghua University] 

**Date:** 24 January 2016, At: 16:58 

_International Journal of Production Research_ , 2015 http://dx.doi.org/10.1080/00207543.2015.1076941 

### **New block properties for flowshop scheduling with blocking and their application in an iterated greedy algorithm** 

Jian-Ya Ding<sup>a</sup> , Shiji Song<sup>a∗</sup> , Jatinder N.D. Gupta<sup>b</sup> , Cheng Wang<sup>a</sup> , Rui Zhang<sup>c</sup> and Cheng Wu<sup>a</sup> 

_aDepartment of Automation, Tsinghua University, Beijing, P.R. China; bCollege of Business Administration, University of Alabama in Huntsville, Huntsville, AL, USA;_<sup>_c_</sup> _School of Economics and Management, Nanchang University, Nanchang, P.R. China_ 

( _Received 10 April 2014; accepted 6 July 2015_ ) 

This paper proposes new block properties for the flowshop scheduling problem with blocking to minimise makespan. A pruning procedure based on these proposed properties is used in the construction phase of an iterated greedy algorithm to decrease the total number of solutions to be examined to find an optimal schedule. Computational results using Taillard’s benchmark problem instances show that the new block properties help to eliminate more ‘unpromising’ solutions than the classic properties. In addition, the effectiveness of the proposed algorithm is verified by comparison with some highperforming algorithms for the considered problem. 

**Keywords:** scheduling; flowshop with blocking; block properties; pruning procedure; iterated greedy algorithm 

#### **1. Introduction** 

Flowshop scheduling problem (FSP) with infinite intermediate storage space has been extensively studied for more than 50 years (Gupta and Stafford 2006). However, with the development of ‘just-in-time’ manufacturing and Kanban control systems, scheduling problems with finite and zero intermediate storage space have received increasing attention (Leisten 1990; Ronconi and Armentano 2001). In the face of zero intermediate storage space (or zero buffer) between successive machines, either the machine must be blocked until the next machine becomes available to process a job or a job must wait before starting its first operation so that it can be continuously processed through the shop. In the former case, the problem is called a flowshop with blocking while in the latter case, the problem is known as a no-wait flowshop (Ding et al. 2015). 

This paper considers the FSP with blocking (or simply a _Blocking Flowshop Scheduling Problem_ , BFSP) to minimise makespan, which is defined as the total elapsed time to process all jobs on all machines. BFSP arises in situations where intermediate storage is not allowed because of technological requirements, such as chemical and pharmaceutical industries (Grabowski and Pempera 2007). Applications of the BFSP can be found in Grabowski and Pempera (2000) and Ronconi (2005). 

ToformallydefinetheBFSP,considerthefollowingscenario:attimezero,eachofthe _n_ jobsfromtheset _N_ = {1 _,_ 2 _, . . . , n_ } is to be processed on a set _M_ = {1 _,_ 2 _, . . . , m_ } of _m_ machines in the same order as given by the indexing of machines. Each job _i_ ∈ _N_ requires a given processing time _pi j_ on every machine _j_ ∈ _M_ . Upon completion of its operation on a machine, a job cannot leave this machine until the next machine is available for processing that job (i.e. a job blocks the machine on which it has just completed its processing until it can start to be processed on the next machine). Other assumptions related to FSPs described by Gupta (1979) and Gupta and Stafford (2006) apply to the problem in this paper as well. Similar to most existing literature addressing this topic, we wish to find a permutation schedule (a schedule with the same job processing order on all machines) which satisfies the above assumptions and meanwhile minimises the maximum completion time (i.e. makespan). 

Let _Di j_ denote the departure time of job _i_ on machine _j_ , with _Di_ 0 denoting the _starting time_ of job _i_ on the first machine. Also, let _�_ be the set of all _n_ ! possible schedules for the BFSP. Then, using the developments in Grabowski and Pempera (2007), the makespan _C_ max _(π)_ for a given schedule _π_ = _(π(_ 1 _), π(_ 2 _), . . . , π(n))_ can be calculated by the following recursive expressions: 

> ∗Corresponding author. Email: shijis@mail.tsinghua.edu.cn 

> © 2015 Taylor & Francis 

_J.-Y. Ding_ et al. 

2 

_Dπ(_ 1 _)_ 0 = 0 _,_ (1) 



<!-- Start of picture text -->
Dπ( 1 ) j = Dπ( 1 ), j −1 + pπ( 1 ) j , j = 1 ,  2 , . . . , m  − 1 , (2)<br><!-- End of picture text -->



<!-- Start of picture text -->
Dπ(i) 0 = Dπ(i −1 ) 1 , i = 2 ,  3 , . . . , n, (3)<br><!-- End of picture text -->



<!-- Start of picture text -->
Dπ(i) j = max{ Dπ(i), j −1 + pπ(i) j , Dπ(i −1 ), j +1} , i = 2 ,  3 , . . . , n, j = 1 ,  2 , . . . , m  − 1 (4)<br><!-- End of picture text -->



<!-- Start of picture text -->
Dπ(i)m = Dπ(i),m −1 + pπ(i)m, i = 2 ,  3 , . . . , n. (5)<br><!-- End of picture text -->

Then, the makespan _C_ max _(π)_ of schedule _π_ ∈ _�_ is: 



<!-- Start of picture text -->
C max (π)  = Dπ(n)m. (6)<br><!-- End of picture text -->

Then the BFSP considered here is to find a permutation schedule _π_<sup>∗</sup> ∈ _�_ such that: 



<!-- Start of picture text -->
C max (π ∗ )  = π min∈ � C max (π). (7)<br><!-- End of picture text -->

The BFSP with more than two machines is known to be NP-hard in the strong sense (Hall and Sriskandarajah 1996). As a result, exact methods such as branch-and-bound algorithm are not computationally capable to solve large-size problems found in practical settings. Therefore, heuristic algorithms are used to find optimal or near-optimal solutions for the BFSP problem. 

Currently available heuristic methods to solve the above-defined BFSP are of two types: constructive and improvement. Constructive methods include Profile Fitting (PF) (McCormick et al. 1989), MinMax (MM) (Ronconi 2004). Improvement algorithms include Hybrid Discrete Differential Evolution (HDDE) (Wang et al. 2010), Tabu Search (TS) (Grabowski and Pempera 2007), Iterated Greedy (IG) (Ribas, Companys, and Tort-Martorell 2011), Three Phase Algorithm (TPA) (Wang et al. 2012), Revised Artificial Immune System (RAIS) (Lin and Ying 2013), and Memetic Algorithm (MA) (Pan et al. 2013). Reviews of existing developments and solution procedures related to BFSP are given by Hall and Sriskandarajah (1996), Wang et al. (2010) and Ribas, Companys, and Tort-Martorell (2011). These reviews show that the computational efficiency of the improvement methods can be enhanced by using well developed _block properties_ that reduce the search space for potential schedules (Grabowski and Pempera 2000, 2007). 

In this paper, therefore, we develop new block properties that include moves between blocks and show that their application in an IG algorithm reduces its computational effort needed to solve the BFSP. The remainder of this paper is organised as follows. Section 2 briefly describes the classic block properties and develops several proposed block properties. Implementation of the proposed block properties in an IG algorithm is discussed in detail in Section 3. The computational results related to the computational efficiency of the proposed block properties in the IG algorithm to solve the BFSP are provided in Section 4. Finally, Section 5 concludes the paper with some fruitful directions for future research. 

#### **2. Block properties** 

Block properties are based on the graph representation of scheduling problems and have been extensively utilised to solve scheduling problems of different types such as: the single-machine (Bozejko 2009; Bozejko, Grabowski, and Wodecki 2006; Grabowski, Nowicki, and Zdrzalka 1986; Wodecki 2009; Zdrzalka and Grabowski 1989), the parallel-machine (Nowicki and Smutnicki 1998), the flowshop (Grabowski and Pempera 2001) and the job-shop (Nowicki and Smutnicki 1996a). Block properties have also been incorporated into many standalone algorithms, such as the NEH (Gao, Chen, and Zhang 2007; Jin, Song, and Wu 2007), TS (Grabowski and Pempera 2007; Grabowski and Wodecki 2004; Nowicki and Smutnicki 1996b), simulated annealing (Jin, Song, and Wu 2009; Zhang and Wu 2011), differential evolution (Qian et al. 2009), PSO (Liu, Wang, and Jin 2008) and the genetic algorithm (Wang, Zhang, and Zheng 2006). 

Originally, only block properties of classic FSP had been developed. Recently, these block properties have been extended to more complex scheduling problems, such as FSPwith mixed no-wait/no-store (Grabowski and Pempera 2000), with buffers (Li and Tang 2005; Nowicki 1999; Smutnicki 1998), or with blocking (Grabowski and Pempera 2007). In this section, we first review the existing classic block properties for the BFSP, and then propose and use new block properties to improve the efficiency of the heuristic algorithms to solve the BFSP. 

#### **2.1** **_Classic block properties_** 

In view of the Max function in Equation (4), the makespan of a schedule for the BFSP can be represented as a critical path in a graph indicating jobs on the horizontal axis and machines on the vertical axis, see Grabowski and Pempera (2000, 2007) for detailed discussion about this issue. As shown in Figure 1, for a BFSP with 15 jobs and 5 machines, we can define 

_International Journal of Production Research_ 

3 



Figure 1. Graph representation model for a BFSP with 15 jobs and 5 machines. 

several types of _job-blocks_ comprising such a critical path and develop _block properties_ to eliminate several blocks from consideration when searching for an optimal solution. 

For a schedule _π_ ∈ _�_ of a BFSP, a graph _G(π)_ = _(S, F_<sup>+</sup> ∪ _F_<sup>0</sup> ∪ _F_<sup>−</sup> _)_ can be constructed by a set of nodes _S_ = _M_ × _N_ and a set of arcs _F_<sup>+</sup> ∪ _F_<sup>0</sup> ∪ _F_<sup>−</sup> , where 



<!-- Start of picture text -->
m<br>F + = � n �{ (( j, k  − 1 ), ( j, k)) } , (8)<br>j =1 k =2<br><!-- End of picture text -->



<!-- Start of picture text -->
m<br>F 0 = � n �{ (( j − 1 , k), ( j, k)) } , (9)<br>j =2 k =1<br><!-- End of picture text -->



<!-- Start of picture text -->
m<br>F − = � n �{ (( j − 1 , k), ( j, k  − 1 )) } . (10)<br>j =2 k =2<br><!-- End of picture text -->

The above equations can be intuitively explained as follows: Equation (8) refers to the machines sequences of jobs, Equation (9) refers to the job sequences of machines and Equation (10) refers to the blocking constraints. 

Each node _( j, k)_ ∈ _G_ has a weight _pπ( j)k_ that represents the processing of job _π( j)_ on machine _k_ . Each arc { _(( j_ − 1 _, k), ( j, k_ − 1 _))_ } ∈ _F_<sup>−</sup> ensures the blocking of job _π( j)_ on machine _k_ if the next machine _k_ + 1 is not free. The arc { _(( j_ − 1 _, k), ( j, k_ − 1 _))_ } ∈ _F_<sup>−</sup> has a weight − _pπ( j_ −1 _)k_ , while the weight of arcs in _F_<sup>+</sup> and _F_<sup>0</sup> are all zero. 

The makespan _C_ max _(π)_ equals to the length of the longest path ( _critical path_ ) in _G(π)_ from node _(_ 1 _,_ 1 _)_ to _(n, m)_ . Let _( j, k)_ be a node in the critical path. Then, the makespan of the schedule _π_ can be calculated using the following procedure: 

#### _Procedure path-detection_ 

_Step 1_ Let _( j, k)_ = _(n, m)_ be the last node in the graph. _Step 2_ Select the immediately previous node _( j, k)_ according to the following equations: 



<!-- Start of picture text -->
⎧ ( j − 1 , k), if Dπ( j)k = Dπ( j −1 )k + pπ( j)k,<br>( j, k)  = ( j, k  − 1 ), if Dπ( j)k = Dπ( j)k −1 + pπ( j)k, (11)<br>⎪⎨<br>⎪⎩ ( j − 1 , k  + 1 ), if Dπ( j)k = Dπ( j −1 ),k +1 .<br><!-- End of picture text -->

_Step 3_ Repeat Steps 1 and 2 until the node _(_ 1 _,_ 1 _)_ is reached. 

To help understand the path detection procedure, we depict in Figure 2 the Gantt chart as well as the critical path of a 4 jobs and 4 machines BFSP. We take node (2,2) as an example to show its previous node in the critical path is obtained by Equation (11). The departure time of each job on each machine can be easily observed from the Gantt chart. It is obvious that D _π(_ 2 _)_ 2̸ = D _π(_ 1 _)_ 2+p _π(_ 2 _)_ 2, D _π(_ 2 _)_ 2̸ = D _π(_ 2 _)_ 1+p _π(_ 2 _)_ 2, and that D _π(_ 2 _)_ 2=D _π(_ 1 _)_ 3. Thus node (1,3) is selected as the intermediate previous node of node (2,2). 

The critical path can be decomposed into several subpaths, according to the three types of arcs defined by Equations (8)–(10). Every subpath (called a block) is composed of consecutive nodes linked by the same type of arcs. However, only 

4 



<!-- Start of picture text -->
Machines.<br>J.-Y. Ding  et al.<br>I$ Gana My fe) (ome)<br>M, ao<br>|)  i Da s Mz<br>M Dayz o T] M; fe) fe)<br>My a 'O 9 0<br>Inca) Inia) Inca) Inca)<br>Gantt Chart Time Critical Path<br><!-- End of picture text -->

Figure 2. Example of Gantt chart and critical path of a BFSP with 4 jobs and 4 machines. 

two types of subpaths containing the nodes linked by arcs in _F_<sup>0</sup> or _F_<sup>−</sup> are considered in the classic block properties. Now we give some definitions about blocks and introduce the classic block properties (Grabowski and Pempera 2007). 

_Definition 1_ (Normal block) A subpath containing the nodes linked by the arcs in _F_<sup>0</sup> is called a normal block of jobs in _π_ , denoted as _N Bl , l_ ∈ _GH_ , where _GH_ = {1 _,_ 2 _, . . . , lN B_ } is the index set of the normal blocks. 

Analogously, we define an anti-block as follows: 

_Definition 2_ (Anti-block) A subpath containing the nodes linked by the arcs in _F_<sup>−</sup> is called an anti-block of jobs in _π_ , denoted as _ABl , l_ ∈ _ST_ , where _ST_ = {1 _,_ 2 _, . . . , l AB_ } is the index set of the anti-blocks. 

The above two types of subpaths form a category called _block_ , denoted by _Bl_ . 

_Definition 3_ (Internal block) The subsequence excluding the first job and the last job in _N Bl_ ( _ABl_ ) is called an internal block of _N Bl_ ( _ABl_ ) and is denoted as _N Bl_<sup>∗(</sup><sup>_AB_</sup> _l_<sup>∗).</sup> 

It is noteworthy that there may exist some blocks of which the corresponding internal blocks are empty. 

_Definition 4_ (T-sequence) A subpath which contains the nodes linked by arcs in _F_<sup>+</sup> is called a T-sequence. 

Based on the graph representation model in Figure 1 and the definitions above, the classic block properties developed by Grabowski and Pempera (2000) can be stated as follows. 

_Property 1_ (Grabowski and Pempera 2000) For any normal block _N Bl_ in _π_ and a processing order _α_ obtained from _π_ by an interchange of jobs from the internal block _N Bl_<sup>∗, we have</sup><sup>_C_max</sup><sup>_(α)_≥</sup><sup>_C_max</sup><sup>_(π)_.</sup> 

_Property 2_ (Grabowski and Pempera 2000) For any anti-block _ABl_ in _π_ and a processing order _α_ obtained from _π_ by an interchange of jobs from the internal block _ABl_<sup>∗, we have</sup><sup>_C_max</sup><sup>_(α)_≥</sup><sup>_C_max</sup><sup>_(π)_.</sup> 

Based on the graph representation model in Figure 1 and the definitions above, the classic block properties lead to the following theorem which provides the necessary condition to obtain a better makespan permutation. 

Theorem 1 (Grabowski and Pempera 2007) _Let π_ ∈ _�_ Tl _be any permutation with normal block N Bl and anti-block ABl . If the permutation β has been obtained from π by an interchange of jobs such that C_ max _(β) < C_ max _(π), then in β_ 

- (i) _at least one job j_ ∈ _N Bl precedes the first job of N Bl , for some l_ ∈ _GH, or_ 

- (ii) _at least one job j_ ∈ _N Bl succeeds the last job of N Bl , for some l_ ∈ _GH, or_ 

- (iii) _at least one job j_ ∈ _ABl precedes the first job of ABl , for some l_ ∈ _ST , or_ 

- (iv) _at least one job j_ ∈ _ABl succeeds the last job of ABl , for some l_ ∈ _ST ._ 

While Theorem 1 provides a necessary condition to obtain an improved solution _β_ from a schedule _π_ , computational experiments show that the average length of blocks is very short. This indicates that better block properties may increase the efficiency of a search algorithm to solve the BFSP. With this in mind, we now propose new block properties for the BSFP that can be used to prune several lengthy partial schedules quite early in the search procedure. 

#### **2.2** **_Proposed block properties_** 

To describe the proposed block properties for the BFSP, let _si_ and _ei_ be the starting machine and the ending machine of the _i_ th block, respectively. Obviously, for a normal block, _si_ = _ei_ , and for an anti-block, _si > ei_ . 

5 

_International Journal of Production Research_ 



Figure 3. Proof of Lemma 1. 

_Definition 5_ (Joined block) The _l_ th block _Bl_ is called a _joined block_ if _el_ −1 = _sl_ , where _e_ 0 = 0. 

Given the above definition, for the _l_ th block { _π(ul_ −1 _), π(ul_ −1 + 1 _), . . . , π(ul )_ }, a set of appropriate positions is defined as follows: 



<!-- Start of picture text -->
{ ul −1 , ul −1 + 1 , . . . , ul − 1} , if the  l th block is joined ,<br>Ul = (12)<br>{ ul −1 + 1 , . . . , ul − 1} , otherwise .<br>�<br><!-- End of picture text -->

Therefore, we can redefine an internal block as follows. 

_Definition 6_ (Internal block) An internal block is a subpath of a block excluding its last node if the block is a joined block, and otherwise, an internal block is defined as a subpath of a block excluding its first and last node. 

With the above definitions, for the construction of blocks in a critical path, we have the following theorem. 

Lemma 1 _At least one critical path exists in which a normal block immediately precedes each anti-block._ 

_Proof_ (By contradiction) Suppose the claim in this lemma is false. Then for every critical path, there must exist an antiblock, immediately before which there is a T-sequence. Let _p_ be such a critical path. Now we consider another path _p_<sup>′</sup> , which contains the same nodes in _p_ except the node connecting anti-block and the T-sequence in _p_ (illustrated in Figure 3). The length of _p_<sup>′</sup> is equal to length of _p_ , so _p_<sup>′</sup> is also a critical path by the definition of critical path. However, in _p_<sup>′</sup> , immediately before every anti-block, there is a normal block, which is a contradiction to the supposition. Hence the claim of Lemma 1 is true. □ 

Let _VGH_ , _VST_ , _VT_ be the set of nodes in internal normal block, internal anti-block and T-sequence, respectively. Let _T_ be set of nodes in the critical path. From the interpretation of Theorem 1, we know that every anti-block must be joined. So 



<!-- Start of picture text -->
VGH � VST � VT =  T,<br>VGH � VST = ∅ , VGH � VT = ∅ , VST � VT = ∅ .<br><!-- End of picture text -->

Thus, | _T_ | = | _VGH_ | + | _VST_ | + | _VT_ |. 

Each node _ni_ ∈ _T_ has a weight _pni_ . Let _W (ni , ni_ +1 _)_ be the weight of the arc connecting nodes _ni_ and _ni_ +1. We have 



<!-- Start of picture text -->
− pni , if  ni ∈ VST ,<br>W (ni , ni +1 )  = (13)<br>0 , otherwise .<br>�<br><!-- End of picture text -->

Given the above definitions and notations, the makespan of _π_ (i.e. the length of the critical path in _G(π)_ ) can be expressed as follows. 



<!-- Start of picture text -->
C max (π)  = � pni + � W (ni , ni +1 )<br>ni ∈ T ni ∈ T<br>= � pni − � pni<br>ni ∈ T ni ∈ VST<br>= � pni + � pni ,<br>ni ∈ VGH ni ∈ VT<br><!-- End of picture text -->

where the last equality follows from the fact that _T_ = _VGH_ � _VST_ � _VT_ and that | _T_ | = | _VGH_ | + | _VST_ | + | _VT_ |. 

6 

_J.-Y. Ding_ et al. 

For a job _π(a)_ , we define the set of moves to the left as _L a(x, y)_ = { _(a, b)_ | _x_ ≤ _b_ ≤ _y, y < a_ } and the set of moves to the right as _Ra(x, y)_ = { _(x, y)_ | _x_ ≤ _b_ ≤ _y, a < x_ }. Defining these moves can help to formulate the shifting operations of a job _π(a)_ between different job blocks. 

_Definition 7_ Let _π_ ∈ _�_ be any permutation corresponding to block _Bl , l_ = 1 _,_ 2 _, . . . , l B_ . Let _πv_ be the permutation obtained by the move _v_ = _(a, b)_ from either _L a(uq_ −1 + 1 _, uq ), a_ ∈ _Uk, q < k_ or _Ra(uq_ −1 _, uq_ − 1 _), a_ ∈ _Uk, q > k_ . Define 



<!-- Start of picture text -->
π(uq − 1 ), if  b ̸ =  uq , q < k or  b ̸ =  uq − 1 , q > k,<br>π 0 =<br>π(a), otherwise.<br>�<br><!-- End of picture text -->

Theorem 2 _Let π_ ∈ _Π be any permutation corresponding to block Bl , l_ = 1 _,_ 2 _, . . . , l B. Let πv be the permutation obtained by the move v_ = _(a, b) from either L a(uq_ −1 + 1 _, uq ), q < k or Ra(uq_ −1 _, uq_ − 1 _), q > k, a_ ∈ _Uk, k_ ∈ _GH, Then C_ max _(πv)_ ≥ _C_ max _(π)_ + _�kq (π), where_ 



<!-- Start of picture text -->
pπ(a)sq − pπ(a)sk , q ∈ GH,<br>�kq (π)  =<br>� pπ 0 eq − pπ(a)sk , q ∈ ST.<br><!-- End of picture text -->

_Proof_ The main idea of the proof is to construct a new path in _G(πv)_ , which is not necessarily a critical path with respect to _πv_ . Thus, by the definition of critical path, the length of this new path gives a lower bound for the makespan of _πv_ . 

The proof for the case _q_ ∈ _GH_ is omitted because it is similar to the proof of Theorem 2 in Grabowski and Pempera (2001). For the case _q_ ∈ _ST_ , we only show the proof for the case _q > k_ and omit the proof for the case _q < k_ as it follows similarly. Let path _p_ = _(B_ 1 _, B_ 2 _, . . . , Bl )_ be a critical path with respect to _π_ . Note that _q > k_ implies that the move is of the type _Ra(uq_ −1 _, uq_ − 1 _)_ . Since _q_ ∈ _ST_ , we consider a new path _p_<sup>′</sup> = _(B_ 1<sup>′</sup><sup>_, B_</sup> 2<sup>′</sup><sup>_, . . . , B_</sup> _l_<sup>′</sup><sup>_)_, where</sup> 



<!-- Start of picture text -->
Bi ′ = Bi , i = 1 ,  2 , . . . , l ; i ̸ =  k, q,<br>Bk ′ =  ((π(uk −1 ), sk), . . . , (π(a  − 1 ), sk), (π(a  + 1 ), sk), . . . , (π(uk), sk)),<br>((π(uq −1 ), sq ), . . . , (π(uq − 1 ), eq ), (π(uq ), eq )), if  b ̸ =  uq − 1 ,<br>B ′<br>q =<br>� ((π(uq −1 ), sq ), . . . , (π(uq − 1 ), eq + 1 ), (π(a), eq ), (π(uq ), eq )), otherwise .<br><!-- End of picture text -->

As to path _p_<sup>′</sup> , _VGH_<sup>′=</sup><sup>_VGH_+ {</sup><sup>_(π_0</sup><sup>_, eq)_} −{</sup><sup>_(π(a), sk)_},</sup><sup>_V_</sup> _T_<sup>′=</sup><sup>_VT_. Thus, the length of path</sup><sup>_p_′,</sup><sup>_L(p_′</sup><sup>_)_is calculated as</sup> follows: 



<!-- Start of picture text -->
L( p ′ )  = � Pni + � Pni<br>ni ∈ VGH ′ ni ∈ VT ′<br>= � Pni + pπ 0 eq − pπ(a)sk + � Pni<br>ni ∈ VGH ni ∈ VT<br>=  C max (π)  + pπ 0 eq − pπ(a)sk .<br><!-- End of picture text -->

By the definition of critical path, _C_ max _(πv)_ ≥ _L( p_<sup>′</sup> _)_ = _C_ max _(π)_ + _pπ_ 0 _eq_ − _pπ(a)sk_ . □ 

In view of Theorem 2, we can obtain a lower bound on the makespan value of the new permutation resulting from interchanging job _a_ to any other position _b_ in the permutation when job _a_ originally lies in a normal block. Next, we show by Theorems 3 and 4 that the lower bound value of the interchanged permutation can also be calculated when job _a_ originally lies in an anti-block or a T-sequence. 

Theorem 3 _Let π_ ∈ _Π be any permutation corresponding to block Bl , l_ = 1 _,_ 2 _, . . . , l B. Let πv be the permutation obtained by the move v_ = _(a, b) from either L a(uq_ −1 + 1 _, uq ), q < k or Ra(uq_ −1 _, uq_ − 1 _), q > k, a_ ∈ _Uk, k_ ∈ _ST . Then, C_ max _(πv)_ ≥ _C_ max _(π)_ + _�kq (π), where_ 



<!-- Start of picture text -->
pπ(a)sq − pπ(uk −1−1 )sk , q ∈ GH, q ̸ =  k  − 1 , or q =  k  − 1 , b ̸ =  uq −1 ,<br>�kq (π)  =<br>� pπ 0 eq − pπ(uk −1−1 )sk , q ∈ ST.<br><!-- End of picture text -->

_Proof_ For simplicity, the theorem is proved only for the case _q_ ∈ _GH_ since the theorem for the case _q_ ∈ _ST_ can be proved similarly. 

_International Journal of Production Research_ 

7 

Assume that _q > k_ , consider a new path _p_<sup>′</sup> = _(B_ 1<sup>′</sup><sup>_, B_</sup> 2<sup>′</sup><sup>_, . . . , B_</sup> _l_<sup>′</sup><sup>_)_in</sup><sup>_G(πv)_, where</sup> 



<!-- Start of picture text -->
Bi ′ = Bi , i = 1 ,  2 , . . . , l ; i ̸ =  k  − 1 , k, q,<br><!-- End of picture text -->



<!-- Start of picture text -->
Bk ′ −1 =  ((π(uk −2 ), sk −1 ), . . . , (π(uk −1 − 1 ), sk −1 )) , (14)<br><!-- End of picture text -->



<!-- Start of picture text -->
Bk ′ =  ((π(uk −1 − 1 ), sk), . . . , (π(uk), ek)) , (15)<br><!-- End of picture text -->



<!-- Start of picture text -->
Bq ′ =  ((π(uq −1 ), sq ), . . . , (π(b), sq ), (π(a), sq ), . . . , (π(uq ), sq )).<br><!-- End of picture text -->

Equations (14) and (15) hold because of Theorem 2. Thus, _VGH_<sup>′=</sup><sup>_VGH_+ {</sup><sup>_(πa, eq)_} −{</sup><sup>_(π(uk_−1 −1</sup><sup>_), sk)_},</sup><sup>_V_</sup> _T_<sup>′=</sup><sup>_VT_. Then</sup> the proof of Theorem 3 is similar to proof of Theorem 2. □ 

Theorem 4 _Let π_ ∈ _Π beanypermutationcorrespondingtoblock Bl , l_ = 1 _,_ 2 _, . . . , l B.Let πv bethepermutationobtained by the move v_ = _(a, b) from either L a(uq_ −1 + 1 _, uq ), q < a or Ra(uq_ −1 _, uq_ − 1 _), q > a, a_ ∈ _V . Let sa and ea be the starting machine and the ending machine of subpath corresponding to π(a). Then we have C_ max _(πv)_ ≥ _C_ max _(π)_ + _�aq (π), where_ 



<!-- Start of picture text -->
pπ(a)sq − ( � i e = a sa pπ(a)i − � i e = a sa +1 pπ(a −1 )i ), q ∈ GH,<br>�aq (π)  =<br>� pπ 0 eq − ( � i e = a sa pπ(a)i − � i e = a sa +1 pπ(a −1 )i ), q ∈ ST.<br><!-- End of picture text -->

_Proof_ For case _q_ ∈ _GH_ , assume that immediately before _π(a)_ there is an anti-block _ABk_ (the other case can be proved similarly). Assume _k < q_ . Consider two subpaths in _p_ , 



<!-- Start of picture text -->
Bk =  ((π(uk −1 ), sk), . . . , (π(uk − 1 ), ek + 1 ), (π(a), sa), (π(a), sa + 1 ), . . . , (π(a), ea)),<br>Bq =  ((π(uq −1 ), sq ), (π(uq −1 + 1 ), sq ), . . . , (π(uq ), sq )).<br><!-- End of picture text -->

A new path _p_<sup>′</sup> is constructed from _p_ by changing these two subpaths in order to represent the following subpaths, 



<!-- Start of picture text -->
Bk =  ((π(uk −1 ), sk), . . . , (π(uk − 1 ), ek + 1 ), (π(uk − 1 ), sa − 2 ), . . . , (π(uk − 1 ), ea)),<br>Bq =  ((π(uq −1 ), sq ), . . . , (π(b), sq ), (π(a), sq ), . . . , (π(uq ), sq )).<br><!-- End of picture text -->

Thus, _VGH_<sup>′=</sup><sup>_VGH_+{</sup><sup>_(πa, sq)_},</sup><sup>_V_</sup> _T_<sup>′=</sup><sup>_VT_+{</sup><sup>_(π(uk_−1</sup><sup>_), i)_|</sup><sup>_i_=</sup><sup>_sa_+1</sup><sup>_, sa_+2</sup><sup>_, . . . , ea_}−{</sup><sup>_(π(uk), i)_|</sup><sup>_i_=</sup><sup>_sa, sa_+1</sup><sup>_, . . . , ea_}.</sup> Then the proof is similar to proof of Theorem 2. □ 

The above theorems indicate that after a permutation _π_ is evaluated, we can get the lower bound for the makespan of permutation _πv_ , which is obtained from _π_ by a move, in _O(_ 1 _)_ computational effort. On the other hand, it takes _O(mn)_ computational effort to calculate accurately the makespan of _πv_ . If the lower bound for _C_ max _(πv)_ is not less than _C_ max _(π)_ , then _πv_ will not be better than _π_ . This knowledge results in significant computational savings for exploring the neighbourhood of _π_ . 

#### **3. Enhanced IG algorithm** 

We now incorporate the proposed properties in the IG algorithm to solve the BFSP. To do so, we first briefly describe the existing IG algorithm and then discuss the components of the enhanced IG algorithm that utilises the proposed block properties. 

#### **3.1** **_Existing IG algorithm_** 

The IG algorithm is an effective local search heuristic recently developed for combinatorial optimisation problems. It has been shown to perform well for solving permutation FSPs (Ruiz and Stutzle 2007, 2008). The IG algorithm consists of two main phases: destruction and construction. In the destruction phase, some elements of the current solution are randomly selected and removed from the current solution. During the construction phase, the removed elements are reinserted into the partial sequence by a NEH-insert procedure one by one, according to the order they are removed. When a complete solution is constructed, an acceptance criterion is applied to decide whether it will replace the current solution or not. This procedure is repeated until predefined termination conditions are met. We will then propose an accelerated IG algorithm with the new block properties embedded in its structure. 

#### **3.2** **_Initial solution_** 

NEH algorithm is a simple constructive heuristic with excellent performance. Therefore, it has been used by many improvement heuristics to generate the initial solution. In the original NEH, jobs are sorted in non-ascending order of their total 

_J.-Y. Ding_ et al. 

8 

processing time. However, in BFSP, larger total processing time leads to higher probability of occurrence of blocking. Thus, a modified NEH algorithm is presented by Wang, Pan, and Suganthan (2010) to solve BFSP with total flowtime criterion, which uses a new priority rule by sequencing jobs in non-descending order of their total processing time. Our proposed algorithm B-IG uses this modified version of the NEH algorithm (called m-NEH) to generate initial solution. 

The steps of the m-NEH algorithm are as follows. 

#### **Algorithm m-NEH: Modified NEH Algorithm** 

_Step 1_ Sort jobs in non-decreasing order of their total processing time, denote the jobs _J_<sup>1</sup> _, J_<sup>2</sup> _, . . . , J_<sup>_n_</sup> according to the sorting. 

_Step 2_ Schedule _J_<sup>1</sup> and _J_<sup>2</sup> so as to minimise the partial makespan. _Step 3_ For _k_ = 3 to _n_ , insert the _k_ th job _J_<sup>_k_</sup> into the partial schedule in the position which minimises the partial makespan. 

#### **3.3** **_Destruction and construction phases_** 

The destruction and construction phases are two central procedures of the IG heuristic. The destruction phase consists of extracting _d_ randomly chosen jobs without repetition from the current solution. The construction phase re-inserts the chosen jobs one at a time in the position that minimises the partial makespan, as is done in Step 3 of NEH. 

In the proposed B-IG, we modified the two phases by only extracting _d_ = 1 job in the destruction phase and re-inserts this job at any position, except its original position in sequence, that minimises the makespan. With the modification, the re-constructed sequence differs from the original one and repeated search is largely avoided, thus the searching efficiency would be enhanced. Then we consider the accelerating techniques for the construction phase. 

Given a partial sequence _π_ with length _l_ and a job _π(a)_ , the sequence by inserting _π(a)_ into position _k_ of _π_ is denoted as _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ . The original NEH Insert Procedure has to test all _l_ + 1 sequences to find the best one, _π_ best. Using the proposed properties, once a solution _π_<sup>_i_</sup> ∈{ _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ } is evaluated, the others in { _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ } can obtain a lower bound in _O(_ 1 _)_ computational effort, and the makespan of _π_<sup>_i_</sup> is an upper bound of _C_ max _(π_ best _)_ . Thus, the ones whose makespan’s lower bound is larger than the upper bound of _C_ max _(π_ best _)_ can be excluded from evaluating. Moreover, when the next one in { _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ } is evaluated, the lower bound of makespan of solutions in { _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ } and upper bound of _C_ max _(π_ best _)_ can be updated. Thus, a pruning procedure based on proposed properties can be introduced into the NEH Insert Procedure to find the best one in { _π_<sup>_k_</sup> _, k_ = 0 _,_ 1 _, . . . , l_ }. In addition, the acceleration techniques for evaluating solutions in the insertion neighborhood by Wang et al. (2010) is adopted here to further reduce the computational efforts. Our procedure can be described as follows, where _U B_ is upper bound of _C_ max _(π_ best _)_ , _L B(π_<sup>_i_</sup> _)_ is the lower bound of _C_ max _(π_<sup>_i_</sup> _)_ and _Evaluated(π_<sup>_i_</sup> _)_ is an evaluated flag of _π_<sup>_i_</sup> . 

#### **Algorithm B-NEH: Block-based NEH Insert Procedure** 

_Step 1_ Set _Evaluated(π_<sup>_k_</sup> _)_ = _False, k_ = 0 _,_ 1 _, . . . , l_ . _Step 2_ Evaluate _π_<sup>0</sup> . Set _Evaluated(π_<sup>0</sup> _)_ = _True_ . Set _U B_ = _C_ max _(π_<sup>0</sup> _)_ , and set current best solution _π_<sup>best</sup> = _π_<sup>0</sup> . Analyse the critical path and blocks of _π_<sup>0</sup> . 

For _i_ = 1 to _l_ , calculate _L B(π_<sup>_i_</sup> _)_ using Theorems 2–4. If _L B(π_<sup>_i_</sup> _)_ ≥ _U B_ , set _Evaluated(π_<sup>_i_</sup> _)_ = _True_ . _Step 3_ For _k_ = 1 : _l_ , _Step 3.1_ If _Evaluated(π_<sup>_k_</sup> _)_ = _True_ , skip Step 3.2–3.4. Else proceed to Step 3.2 _Step 3.2_ Evaluated _π_<sup>_k_</sup> . Set _Evaluated(π_<sup>_k_</sup> _)_ = _True_ . Analyse the critical path and blocks of _π_<sup>_k_</sup> . _Step 3.3_ ( **Update UB** ) If _C_ max _(π_<sup>_k_</sup> _) < U B_ , set _U B_ = _C_ max _(π_<sup>_k_</sup> _)_ and update the current best solution: _π_<sup>best</sup> = _π_<sup>_k_</sup> . _Step 3.4_ ( **Update LB** ) For _i_ = _k_ + 1 to _l_ , calculate the new lower bound _L B_<sup>′</sup> _(π_<sup>_i_</sup> _)_ of the makespan of _π_<sup>_i_</sup> . Set _L B(π_<sup>_i_</sup> _)_ = max{ _L B(π_<sup>_i_</sup> _), L B_<sup>′</sup> _(π_<sup>_i_</sup> _)_ }. If _L B(π_<sup>_i_</sup> _)_ ≥ _U B_ , set _Evaluated(π_<sup>_i_</sup> _)_ = _True_ . 

#### **3.4** **_Acceptance criterion and stopping rule_** 

The proposed B-IG adopts a simple acceptance criterion inspired by the mechanism of simulated annealing algorithm (Osman and Potts 1989): a better solution is accepted with probability 1 and a worse solution is accepted with a probability given by exp{[ _C_ max _(π)_ − _C_ max _(π_<sup>′</sup> _)_ ] _/T_ }, where _C_ max _(π)_ and _C_ max _(π_<sup>′</sup> _)_ are the makespan of the current and candidate solution, respectively, and _T_ is a control temperature that gradually decreases in the searching progress. Following the suggestions of <u>�</u> _im_ =1 <u>�</u> _nj_ =1<sup>_pi j_</sup> Osman and Potts (1989), the initial temperature is set at _Ti_ = _Tc_ · 10· _n_ · _m_ and the final temperature is set at _T f_ = 1, 

_International Journal of Production Research_ 

9 

where _Tc_ is a constant, which has to be tuned. After parameter calibration, we set parameter _Tc_ = 0 _._ 5 throughout the computational analysis. 

Maximum execution time _t_ max is adopted as the stopping rule in our algorithm, while the amount of iterations is unknown. However, we notice that each iteration consumes similar computation efforts and the total number of iterations can be estimated during the execution of the algorithm. Therefore, we propose an adaptive temperature control method to determine the temperature decreasing rate during the algorithm’s executing process as follows. 

In the _(k_ + 1 _)_ th iteration, the temperature decreases by 



<!-- Start of picture text -->
�T (k)  = t (k) · T (k)  − T f (16)<br>k t max − t (k)<br><!-- End of picture text -->

compared with the temperature in the _k_ th iteration, where _t (k)_ is the algorithm execution time of the previous _k_ iterations. In the equation,<sup>_t_</sup><sup>_<u>(</u>_</sup> _k_<sup>_k)_</sup> is the estimated time consumption for each iteration and _t_<sup>_T_</sup> max<sup>_<u>(k</u>_</sup> −<sup>_<u>)</u>_−</sup> _t_<sup>_T_</sup> _(k_<sup>_<u>f</u>_</sup> _)_<sup>is the expected temperature decreasing</sup> rate. The temperature updating equation is given by 



<!-- Start of picture text -->
T ( 1 )  =  Ti<br>T (k  + 1 )  =  T (k)  − �T (k) (17)<br><!-- End of picture text -->

With this control strategy, the temperature decreases at a suitable speed under different maximum execution time, hence leading to a good balance between exploitation and exploration in the progress of searching for optimal or near-optimal solutions. 

#### **3.5** **_Proposed IG algorithm with block properties_** 

The steps of our proposed IG algorithm using our proposed block properties can be stated as follows. 

#### **Algorithm B-IG: Iterated Greedy Algorithm with Block Properties** 

_Step 1_ (Generate initial solution) Generate initial solution by the variant NEH algorithm, denoted by _π_ . Set the iteration <u>�</u> _im_ =1 <u>�</u> _nj_ =1<sup>_pi, j_</sup> number _k_ := 0 and initial temperature _T_ := _Tc_ · 10· _n_ · _m_ . 

_Step 2_ While the stopping rule is not met, do 

_Step 2.1_ (Destruction phase) Randomly select and remove one job from _π_ . Denote the remaining partial sequence by _πD_ and the removed job by _JR_ . 

_Step 2.2_ (Construction phase) Use the Block-based NEH Insert Procedure to find the best position of inserting _JR_ into the partial schedule _πD_ (except for its original position in _π_ ), denote the yield sequence by _π_<sup>′</sup> . _Step 2.3_ (Acceptance) Set _π_ = _π_<sup>′</sup> with probability _p_ , 



<!-- Start of picture text -->
1 , if  �<  0 ,<br>p =<br>e − �/T , if  � ≥ 0 ,<br>�<br><!-- End of picture text -->

where _�_ = _C_ max _(πv)_ − _C_ max _(πcur )_ . 

_Step 2.4_ Set _k_ ← _k_ + 1 and record the algorithm’s present running time as _t (k)_ and update temperature _T_ according to the following equation. 



<!-- Start of picture text -->
T ← T − t (k) · T − T f<br>k t max − t (k) .<br><!-- End of picture text -->

_Step 3_ Output the best solution found. 

#### **4. Experimental results** 

To show the effectiveness of the proposed properties, computational experiments are provided based on the well-known Taillard benchmark problem instances (Taillard 1993) assuming the job blocking constraint is active. This benchmark dataset consists of 120 instances with 12 different sizes, ranging from 20 jobs and 5 machines to 500 jobs and 20 machines. For each size, 10 instances are provided. All algorithms were coded in Visual C++, performed on a PC with Intel Core(TM) CPU at 3.20GHz. 

_J.-Y. Ding_ et al. 



<!-- Start of picture text -->
10 J.-Y. Ding  et al.<br>(a) (b)<br><!-- End of picture text -->



Figure 4. Information about blocks. (a) Average number of blocks under different instance scale. (b) Average length of blocks under different instance scale. 

Table 1. Average number of eliminated solutions in an NEH-Insertion procedure. 

||Proposed properties|Classic properties|Ratio|
|---|---|---|---|
|20×5|7_._30|3_._73|1_._96|
|20×10|6_._48|3_._69|1_._76|
|20×20|5_._82|3_._62|1_._61|
|50×5|23_._70|3_._79|6_._25|
|50×10|19_._05|3_._74|5_._09|
|50×20|18_._80|3_._71|5_._07|
|100×5|49_._70|4_._10|12_._13|
|100×10|45_._40|4_._02|11_._30|
|100×20|41_._60|3_._98|10_._45|
|200×10|99_._20|4_._23|23_._46|
|200×20|91_._60|4_._10|22_._33|
|500×20|245_._50|4_._40|55_._80|
|Average|54_._51|3_._93|13_._10|



#### **4.1** **_Effectiveness of new block properties_** 

In order to evaluate the new block properties, computational experiments were conducted to identify the contribution of the classic properties (i.e. Theorem 1 in the previous section) and the proposed ones in solving the considered problem. By comparing their contributions, the effectiveness of the new properties was revealed. Firstly, some information about the blocks was collected. The computations were conducted as follows. For each Taillard’s instance, the proposed B-IG algorithm was executed 10 times. In the running process of the algorithm, the average number as well as the average length of blocks of the generated solutions were recorded. These statistics help explain the differences in the effectiveness of the classic and proposed properties for solving the considered problem. 

Figure 4 shows the information about blocks under different instance scale. As revealed in Figure 4(a), the average number of blocks is directly related to the number of jobs. More precisely, the increase in number of jobs leads to an increase in number of blocks, while an increase in the number of machines does not affect the number of blocks significantly. In addition, it can be calculated from the statistics that the number of blocks is comparable to the number of jobs, where the ratio of number of blocks to the number of jobs varies slightly between 0.638 and 0.698 under different problem scale settings. This indicates that the average length of blocks tends to be very short. Figure 4(b) shows the average length of blocks. According to this figure, the value of average length ranges from 2.37 to 2.56 for different scaled instances.<sup>1</sup> Since the classic properties only considers interchange of jobs within a block, they do not eliminate the candidate solutions effectively under the condition that each block only contains no more than three jobs on average. 

_International Journal of Production Research_ 

11 

Table 2. Performance comparisons on Taillard’s benchmarks (ARPD). 

||HDDE|IG|TPA|RAIS|MA|TPA<sup>∗</sup>|MA<sup>∗</sup>|B-IG|
|---|---|---|---|---|---|---|---|---|
|20×5|0_._00|0_._05|0_._40|0_._00|0_._03|0_._02|0_._00|0_._00|
|20×10|0_._00|0_._10|0_._21|0_._00|0_._01|0_._01|0_._00|0_._00|
|20×20|0_._00|0_._03|0_._03|0_._00|0_._01|0_._00|0_._00|0_._00|
|50×5|0_._34|0_._65|1_._46|−0_._19|0_._25|0_._17|0_._17|−**0**_._**19**|
|50×10|0_._21|0_._52|1_._07|−0_._20|0_._18|0_._08|−0_._02|−**0**_._**42**|
|50×20|0_._17|0_._41|0_._76|0_._00|0_._16|−0_._09|−0_._11|−**0**_._**39**|
|100×5|1_._10|1_._31|1_._71|−**0**_._**82**|−0_._06|0_._18|−0_._13|−0_._72|
|100×10|0_._55|1_._22|0_._59|−0_._88|−0_._42|−0_._07|−0_._72|−**1**_._**35**|
|100×20|0_._23|0_._97|0_._07|−0_._71|−0_._35|−0_._43|−**1**_._**42**|−1_._36|
|200×10|1_._44|1_._84|1_._22|−0_._33|−0_._38|−0_._24|−0_._60|−**1**_._**15**|
|200×20|0_._73|1_._14|0_._03|−0_._63|−0_._45|−0_._65|−0_._89|−**1**_._**76**|
|500×20|1_._46|1_._58|−0_._05|−0_._02|−2_._30|−0_._89|−**2**_._**72**|−2_._20|
|Average|0_._52|0_._82|0_._65|−0_._31|−0_._28|−0_._18|−0_._54|−**0**_._**80**|



Table 1 shows the average number of eliminated solutions in the NEH-Insertion procedure by using the classic properties and using the proposed properties. As shown in this table, the total average number of eliminated solutions by the proposed properties is 54.51 while the corresponding value by the classic properties is only 3.93. This result suggests great advantage of the new properties compared with the classic ones. 

#### **4.2** **_Comparison with existing algorithms_** 

We test the performance of the proposed B-IG algorithm by comparing it to some high-performing algorithms for BFSP with respect to the makespan criterion. Wang et al. (2010) showed that their HDDE algorithm outperformed the other previous heuristics for the considered problem. Ribas, Companys, and Tort-Martorell (2011) compared their proposed algorithm IG with HDDE, and reported that their algorithm is superior to HDDE. More recently, Wang et al. (2012) proposed a TPA, Lin and Ying (2013) proposed a RAIS algorithm and Pan et al. (2013) proposed a MA for the problem considered. All of the three algorithms validated their effectiveness by comparing the computation results with the previous algorithms. The above mentioned five algorithms constitute the state-of-the-art methods for solving the BFSP in makespan minimisation. Therefore, we compare the proposed B-IG algorithm by computational tests with the above-mentioned approaches to test its effectiveness. To analyse the experimental results, the _average relative percentage deviation_ (ARPD) is calculated for each algorithm and defined as follows. 



<!-- Start of picture text -->
ARPD = 10 1 �10 100 · (C A i C − RC R i ) , (18)<br>i =1 i<br><!-- End of picture text -->

where _Ci_<sup>_A_isthemakespanfoundby algorithm A and</sup><sup>_C R_</sup> _i_<sup>istheupperboundsolutionprovidedbyRibas,Companys, and</sup> Tort-Martorell (2011). The smaller the value of ARPD, the better algorithm A performs. 

To make a fair comparison, we re-implemented the HDDE algorithm and IG algorithm on the same PC and tested their performance against the proposed B-IG algorithm. For each instance, 10 replications were conducted. The performance of the other compared algorithms (TPA, RAIS and MA) is taken from the literature: the result of TPA algorithm from Wang et al. (2012), RAIS algorithm from Lin and Ying (2013) and MA algorithm from Pan et al. (2013). Note that these compared algorithms considered different stopping rule: RAIS, HDDE, IG and the proposed B-IG were allowed to run for 100 · _n_ · _m_ milliseconds, while MA was executed for 15 · _n_ · _m_ milliseconds and TPA terminated when reaching 50,000 iterations in their original paper. To make the comparison results more convincing, we re-implemented the MA and TPA algorithms and set their maximum execution time also at 100 · _n_ · _m_ ms. To distinguish from their original versions, the two re-implemented algorithms are denoted by MA<sup>∗</sup> and TPA<sup>∗</sup> , respectively. 

Table 2 depicts the ARPD values for the proposed algorithm and the other tested algorithms. Compared with the average ARPD values of 0 _._ 52, 0 _._ 82, 0 _._ 65, −0 _._ 31, −0 _._ 28, −0 _._ 18 and −0 _._ 54 obtained by the HDDE, IG, TPA, RAIS, MA, TPA<sup>∗</sup> and MA<sup>∗</sup> algorithms, the B-IG algorithm achieves the best ARPD value of −0 _._ 80. More importantly, the B-IG algorithm shows to be very effective for the middle- and large-scale instances as revealed from the table. This result can be explained by the fact that a large number of candidate solutions are eliminated by utilising the new block properties, and thus the search direction is guided to more promising regions in the solution space. 

_J.-Y. Ding_ et al. 

12 

Table 3. New upper bound solutions for Taillard’s benchmarks. 

|Instance|Best|Source|Instance|Best|Source|Instance|Best|Source|
|---|---|---|---|---|---|---|---|---|
|Ta1|1374|1,2,3,4,5|Ta41|**3624**|5|Ta81|**7717**|5|
|Ta2|1408|1,2,3,4,5|Ta42|3486|4|Ta82|**7776**|5|
|Ta3|1280|1,2,3,4,5|Ta43|**3477**|5|Ta83|**7734**|5|
|Ta4|1448|1,2,3,4,5|Ta44|**3652**|5|Ta84|**7750**|5|
|Ta5|1341|1,2,3,4,5|Ta45|**3622**|5|Ta85|**7762**|5|
|Ta6|1363|1,2,3,4,5|Ta46|3576|3|Ta86|**7806**|5|
|Ta7|1381|1,2,3,4,5|Ta47|**3674**|5|Ta87|**7879**|5|
|Ta8|1379|1,2,3,4,5|Ta48|**3565**|5|Ta88|**7919**|5|
|Ta9|1373|1,2,3,4,5|Ta49|**3521**|5|Ta89|**7843**|5|
|Ta10|1283|1,2,3,4,5|Ta50|**3612**|5|Ta90|**7872**|5|
|Ta11|1698|1,2,3,4,5|Ta51|4479|4|Ta91|**13,275**|5|
|Ta12|1833|1,2,3,4,5|Ta52|**4262**|5|Ta92|**13,169**|5|
|Ta13|1659|1,2,3,4,5|Ta53|4261|4,5|Ta93|**13,285**|5|
|Ta14|1535|1,2,3,4,5|Ta54|**4338**|5|Ta94|**13,233**|5|
|Ta15|1617|1,2,3,4,5|Ta55|**4255**|5|Ta95|**13,220**|5|
|Ta16|1590|1,2,3,4,5|Ta56|**4276**|5|Ta96|**12,975**|5|
|Ta17|1622|1,2,3,4,5|Ta57|**4293**|5|Ta97|**13,461**|5|
|Ta18|1731|1,2,3,4,5|Ta58|**4308**|5|Ta98|**13,359**|5|
|Ta19|1747|1,2,3,4,5|Ta59|**4309**|5|Ta99|**13,165**|5|
|Ta20|1782|1,2,3,4,5|Ta60|4404|2|Ta100|**13,202**|5|
|Ta21|2436|1,2,3,4,5|Ta61|6102|3|Ta101|**14,620**|5|
|Ta22|2234|1,2,3,4,5|Ta62|5979|3|Ta102|**14,818**|5|
|Ta23|2479|1,2,3,4,5|Ta63|5883|3|Ta103|**14,896**|5|
|Ta24|2348|1,2,3,4,5|Ta64|5702|3|Ta104|**14,864**|5|
|Ta25|2435|1,2,3,4,5|Ta65|**5934**|5|Ta105|**14,716**|5|
|Ta26|2383|1,2,3,4,5|Ta66|**5787**|5|Ta106|**14,825**|5|
|Ta27|2390|1,2,3,4,5|Ta67|5936|3|Ta107|**14,851**|5|
|Ta28|2328|1,2,3,4,5|Ta68|5835|3|Ta108|**14,874**|5|
|Ta29|2363|1,2,3,4,5|Ta69|6066|4|Ta109|**14,777**|5|
|Ta30|2323|1,2,3,4,5|Ta70|6125|3|Ta110|**14,761**|5|
|Ta31|**2985**|5|Ta71|**6975**|5|Ta111|35,677|4|
|Ta32|3191|3,5|Ta72|**6673**|5|Ta112|35,953|4|
|Ta33|3007|3|Ta73|**6836**|5|Ta113|35,732|4|
|Ta34|**3122**|5|Ta74|**7091**|5|Ta114|**35,030**|5|
|Ta35|3143|3|Ta75|**6745**|5|Ta115|35,774|4|
|Ta36|**3163**|5|Ta76|**6562**|5|Ta116|35,948|4|
|Ta37|**3012**|5|Ta77|**6728**|5|Ta117|35,631|4|
|Ta38|3058|3|Ta78|**6767**|5|Ta118|35,943|4|
|Ta39|**2903**|5|Ta79|**6971**|5|Ta119|35,658|4|
|Ta40|**3103**|5|Ta80|**6844**|5|Ta120|**36,004**|5|



Considering that the TPA and MA algorithms consume less computational effort than the proposed B-IG algorithm, we conclude that the proposed method is very competitive with the state-of-the-art algorithms for the BFSP to minimise makespan. 

Since the proposed B-IG algorithm is competitive with other high-performing algorithms, new upper bounds solutions were discovered in the computation tests. These solutions are reported in Table 3, where the improved solutions given by the proposed algorithm are in bold font and the original upper bounds are taken from Ribas, Companys, and Tort-Martorell (2011), Wang et al. (2012), Lin and Ying (2013) and Pan et al. (2013). The numbers ‘1’,‘2’,‘3’,‘4’ and ‘5’ in the _Source_ column represents the method adopted to obtain the best solution: number ‘1’indicates the IG algorithm by Ribas, Companys, and Tort-Martorell (2011), number ‘2’ indicates the TPA algorithm by Wang et al. (2012), number ‘3’ indicates the RAIS algorithm by Lin and Ying (2013), number ‘4’ indicates the MA algorithm by Pan et al. (2013), and number ‘5’ indicates the proposed B-IG algorithm. As revealed in the table, a total of 65 new best solutions are obtained by the proposed B-IG algorithm for the considered problem in the Taillard data set. 

_International Journal of Production Research_ 

13 

The above computational results clearly validate the effectiveness of the proposed B-IG algorithm and show that the proposed algorithm is very competitive with other high-performing algorithms in this research area. 

#### **5. Conclusions** 

In this paper, we investigated some new block properties of the FSP with blocking. When a solution is evaluated, these properties can give a lower bound of its neighbour’s makespan. If this lower bound is not less than the current solution’s makespan, this neighbor is not better than current solution, leading to acceleration in neighbourhood exploration. We showed that these properties can be implemented in an IG algorithm to reduce its computational time since these properties can be tested in _O(_ 1 _)_ computational effort. Results of the computational experiments on Taillard’s benchmark problem instances show that the proposed properties can eliminate more candidate solutions than the classic properties. In addition, the proposed algorithm is very competitive when compared with other high-performing approaches to solve the considered problem. While not attempted in this paper, the proposed properties can also be used to accelerate several other meta-heuristics like TS and genetic algorithms. 

Several issues are worthy of future investigations. First, it would be interesting to implement proposed block properties in other meta-heuristics to improve their efficiency and effectiveness. Second, since local search procedure consumes the most computational effort in many state-of-the-art algorithms, it is meaningful to consider more powerful acceleration method for the evaluation of different local search process. 

#### **Disclosure statement** 

No potential conflict of interest was reported by the authors. 

#### **Funding** 

This paper is supported by the National Natural Science Foundation of China [grant number 61273233] and [grant number 61473141]; the Research Foundation for the Doctoral Program of Higher Education [grant number 20120002110035] and [grant number 20130002130010]. 

#### **Note** 

1. Itisnoteworthythatexperimentalevaluationswereconductedundersomeotherrandomlygeneratedinstanceswithdifferentprocessing time distributions, such as uniform distribution U(1,999) and normal distribution N(100,20). However, similar average length results were obtained under different processing time distribution settings. This fact suggests that the (short) length of blocks is not due to the specific structure of processing times in the Taillard test instances (U(1,99) distribution) 

#### **References** 

- Bozejko, W. 2009. “Parallel Path Relinking Method for the Single Machine Total Weighted Tardiness Problem with Sequence-dependent Setups.” _Journal of Intelligent Manufacturing_ 21 (6): 777–785. 

- Bozejko, W., J. Grabowski, and M. Wodecki. 2006. “BlockApproach-tabu SearchAlgorithm for Single Machine Total Weighted Tardiness Problem.” _Computers and Operations Research_ 50: 1–14. 

- Ding, J., S. Song, R. Zhang, J. N. Gupta, and C. Wu. 2015. “Accelerated Methods for Total Tardiness Minimisation in No-wait Flowshops.” _International Journal of Production Research_ 53 (4): 1002–1018. 

- Gao, S. W., L. Chen, and W. D. Zhang. 2007. “An Enhanced NEH Method in Solving Permutation Flow Shop Problem.” _Journal of Shanghai Jiaotong University (Science)_ E-12 (1):47–52. 

- Grabowski, J., E. Nowicki, and S. Zdrzalka. 1986. “ABlockApproach for Single-machine Scheduling with Release Dates and Due Dates.” _European Journal of Operational Research_ 26: 278–285. 

- Grabowski, J., and J. Pempera. 2000. “Sequencing of Jobs in Some Production System.” _European Journal of Operational Research_ 125 (3): 535–550. 

- Grabowski, J., and J. Pempera. 2001. “New Block Properties for the Permutation Flowshop Problem with Application in Tabu Search.” _Journal of the Operational Research Society_ 52: 210–220. 

- Grabowski, J., and J. Pempera. 2007. “The Permutation Flow Shop Problem with Blocking:ATabu SearchApproach.” _Omega_ 35: 302–311. 

- Grabowski, J., and M. Wodecki. 2004. “A Very Fast Tabu Search Algorithm for the Permutation Flow Shop Problem with Makespan Criterion.” _Computers and Operations Research_ 31 (11): 1891–1909. 

Gupta, J. N. D. 1979. “A Review of Flowshop Scheduling Research.” In _Disaggregation Problems in Manufacturing and Service Organizations_ , edited by L. P. Ritzman, L. J. Krawjeski, W. L. Berry, S. M. Goodman, S. T. Hardy and L. D. Vitt, 363–388. The Hague: Martin Nijhoff. 

_J.-Y. Ding_ et al. 

14 

Gupta, J. N. D., and E. F. Stafford. 2006. “Flowshop Scheduling Research after Five Decades.” _European Journal of Operational Research_ 169: 699–711. 

Hall, N. G., and C. Sriskandarajah. 1996. “A Survey of Machine Scheduling Problems with Blocking and No-wait in Process.” _Operations Research_ 44 (3):510–525. 

Jin, F., S. J. Song, and C. Wu. 2007. “An Improved Version of the NEHAlgorithm and itsApplication to Large-scale Flow-shop Scheduling Problems.” _IIE Transcations_ 39: 229–234. 

Jin, F., S. J. Song, and C. Wu. 2009. “A Simulated Annealing Algorithm for Single Machine Scheduling Problems with Family Setups.” _Computers and Operations Research_ 36: 2133–2138. 

Leisten, R. 1990. “Flowshop Sequencing Problems with Limited Buffer Storage.” _International Journal of Production Research_ 28 (22): 2085–2100. 

Li, S. H., and L. X. Tang. 2005. “A Tabu Search Algorithm Based on New Block Properties and Speed-up Method for Permutation Flow-shop with Finite Intermediate Storage.” _Journal of Intelligent Manufacturing_ 16: 463–477. 

Lin, S. W., and K. C. Ying. 2013. “Minimizing Makespan in a Blocking Flowshop Using a Revised Artificial Immune System Algorithm.” _Omega_ 41 (2): 383–389. 

Liu, B., L. Wang, and Y. H. Jin. 2008. “An Effective Hybrid PSO-based aLgorithm for Flow Shop Scheduling with Limited Buffers.” _Computers and Operations Research_ 35: 2791–2806. 

McCormich, S. T., M. L. Pinedo, S. Shenker, and B. Wolf. 1989. “Sequencing in an Assembly Line with Blocking to Minimize Cycle Time.” _Operations Research_ 28: 925–936. 

Nowicki, E. 1999. “The Permutation Flow Shop with Buffers: A Tabu Search Approach.” _European Journal of Operational Research_ 116 (1): 205–219. 

Nowicki, E., and C. Smutnicki. 1996a. “AFast Taboo SearchAlgorithm for the Job Shop Problem.” _Management Science_ 42 (6): 797–813. Nowicki, E., and C. Smutnicki. 1996b. “A Fast Tabu Search Algorithm for the Permutation Flow Shop Problem.” _European Journal of Operational Research_ 91: 160–175. 

Nowicki, E., and C. Smutnicki. 1998. “The Flow Shop with Parallel Machines:ATabu SearchApproach.” _European Journal of Operational Research_ 106: 226–253. 

Osman, I. H., and C. N. Potts. 1989. “Simulated Annealing for Permutation Flow-shop Scheduling.” _Omega_ 17 (6): 551–557. 

Pan, Q. K., L. Wang, H. Y. Sang, J. Q. Li, and M. Liu. 2013. “AHigh Performing MemeticAlgorithm for the Flowshop Scheduling Problem with Blocking.” _IEEE Transactions on Automation Science and Engineering_ 10 (3): 741–756. 

Qian, B., L. Wang, D. X. Huang, and X. Wang. 2009. “An Effective Hybrid DE-based Algorithm for Flow Shop Scheduling with Limited Buffers.” _International Journal of Production Research_ 47 (1): 1–24. 

Ribas, I., R. Companys, and X. Tort-Martorell. 2011. “An Iterated GreedyAlgorithm for the Flowshop Scheduling Problem with Blocking.” _Omega_ 39: 293–301. 

Ronconi, D. P. 2004. “A Note on Constructive Heuristics for the Flowshop Problem with Blocking.” _International Journal of Production Economics_ 87: 39–48. 

Ronconi, D. P. 2005. “A Branch-and-bound Algorithm to Minimize the Makespan in a Flowshop with Blocking.” _Annals of Operations Research_ 138: 53–65. 

Ronconi,D.P.,andV.A.Armentano.2001.“LowerBoundingSchemesforFlowshopswithBlockingIn-process.” _JournaloftheOperational Research Society_ 52: 1289–1297. 

Ruiz, R., and T. Stutzle. 2007. “A Simple and Effective Iterated Greedy Algorithm for the Permutation Flowshop Scheduling Problem.” _European Journal of Operational Research_ 177: 2033–2049. 

Ruiz, R., and T. Stutzle. 2008. “An Iterated Greedy heuristic for the Sequence Dependent Setup Times Flowshop Problem with Makespan and Weighted Tardiness Objectives.” _European Journal of Operational Research_ 187: 1143–1159. 

Smutnicki, C. 1998. “A Two-machine Permutation Flow Shop Scheduling Problem with Buffers.” _OR Spektrum_ 20 (4): 229–235. 

Taillard, E. 1993. “Benchmarks for Basic Scheduling Problems.” _European Journal of Operational Research_ 64: 278–285. 

Wang, L., Q. K. Pan, and P. N. Suganthan. 2010. “Minimizing the Total Flow Time in a Flow Shop with Blocking by Using Hybrid 

Harmony Search Algorithms.” _Expert Systems with Applications_ 37: 7929–7936. 

Wang, L., Q. K. Pan, P. N. Suganthan, W. H. Wang, and Y. M. Wang. 2010. “A Novel Hybrid Discrete Differential Evolution Algorithm for Blocking Flow Shop Scheduling Problem.” _Computers and operations Research_ 37: 509–520. 

Wang, C., S. Song, J. N. D. Gupta, and C. Wu. 2012. “A Three-phase Algorithm for Flowshop Scheduling with Blocking to Minimize Makespan.” _Computers and Operations Research_ 39 (11): 2880–2887. 

Wang, L., L. Zhang, and D. Z. Zheng. 2006. “An Effective Hybrid Genetic Algorithm for Flow Shop Scheduling with Limited Buffers.” _Computers and Operations Research_ 33: 2960–2971. 

Wodecki, M. 2009. “ABlockApproach to Earliness-tardiness Scheduling Problem.” _The International Journal of Advanced Manufacturing Technology_ 40: 797–807. 

Zdrzalka, S., and J. Grabowski. 1989. “An Algorithm for Single Machine Sequencing with Release Dates to Minimize Maximum Cost.” _Discrete Applied Mathematics_ 23: 73–89. 

Zhang, R., and C. Wu. 2011. “A Simulated Annealing Algorithm Based on Block Properties for the Job Shop Scheduling Problem with Total Weighted Tardiness Objective.” _Computers and Operations Research_ 38: 854–867. 

View publication stats 

