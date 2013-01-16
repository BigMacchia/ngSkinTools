//Maya ASCII 2011 scene
//Name: normalization.ma
//Last modified: Tue, Nov 01, 2011 10:15:13 PM
//Codeset: 1257
requires maya "2011";
currentUnit -l centimeter -a degree -t film;
fileInfo "application" "maya";
createNode transform -s -n "persp";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 18.088717821271992 17.473059185329745 11.90889226268115 ;
	setAttr ".r" -type "double3" -41.138352729602445 49.399999999999409 -2.4436698605133695e-015 ;
createNode camera -s -n "perspShape" -p "persp";
	setAttr -k off ".v" no;
	setAttr ".fl" 34.999999999999993;
	setAttr ".coi" 31.020570015924129;
	setAttr ".imn" -type "string" "persp";
	setAttr ".den" -type "string" "persp_depth";
	setAttr ".man" -type "string" "persp_mask";
	setAttr ".hc" -type "string" "viewSet -p %camera";
createNode transform -s -n "top";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 100.1 0 ;
	setAttr ".r" -type "double3" -89.999999999999986 0 0 ;
createNode camera -s -n "topShape" -p "top";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "top";
	setAttr ".den" -type "string" "top_depth";
	setAttr ".man" -type "string" "top_mask";
	setAttr ".hc" -type "string" "viewSet -t %camera";
	setAttr ".o" yes;
createNode transform -s -n "front";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 0 0 100.1 ;
createNode camera -s -n "frontShape" -p "front";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "front";
	setAttr ".den" -type "string" "front_depth";
	setAttr ".man" -type "string" "front_mask";
	setAttr ".hc" -type "string" "viewSet -f %camera";
	setAttr ".o" yes;
createNode transform -s -n "side";
	setAttr ".v" no;
	setAttr ".t" -type "double3" 100.1 0 0 ;
	setAttr ".r" -type "double3" 0 89.999999999999986 0 ;
createNode camera -s -n "sideShape" -p "side";
	setAttr -k off ".v" no;
	setAttr ".rnd" no;
	setAttr ".coi" 100.1;
	setAttr ".ow" 30;
	setAttr ".imn" -type "string" "side";
	setAttr ".den" -type "string" "side_depth";
	setAttr ".man" -type "string" "side_mask";
	setAttr ".hc" -type "string" "viewSet -s %camera";
	setAttr ".o" yes;
createNode transform -n "testMesh";
	setAttr -l on ".tx";
	setAttr -l on ".ty";
	setAttr -l on ".tz";
	setAttr -l on ".rx";
	setAttr -l on ".ry";
	setAttr -l on ".rz";
	setAttr -l on ".sx";
	setAttr -l on ".sy";
	setAttr -l on ".sz";
createNode mesh -n "testMeshShape" -p "testMesh";
	setAttr -k off ".v";
	setAttr -s 4 ".iog[0].og";
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".clst[0].clsn" -type "string" "colorSet1";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
createNode mesh -n "testMeshShapeOrig" -p "testMesh";
	setAttr -k off ".v";
	setAttr ".io" yes;
	setAttr ".vir" yes;
	setAttr ".vif" yes;
	setAttr ".uvst[0].uvsn" -type "string" "map1";
	setAttr -s 16 ".uvst[0].uvsp[0:15]" -type "float2" 0 0 0.32664019 0 
		0.65328038 0 0.97992057 0 0 0.33333334 0.32664019 0.33333334 0.65328038 0.33333334 
		0.97992057 0.33333334 0 0.66666669 0.32664019 0.66666669 0.65328038 0.66666669 0.97992057 
		0.66666669 0 1 0.32664019 1 0.65328038 1 0.97992057 1;
	setAttr ".cuvs" -type "string" "map1";
	setAttr ".dcc" -type "string" "Ambient+Diffuse";
	setAttr ".covm[0]"  0 1 1;
	setAttr ".cdvm[0]"  0 1 1;
	setAttr -s 16 ".vt[0:15]"  -4.6870656 -1.0620633e-015 4.7831078 -1.5623553 
		-1.0620633e-015 4.7831078 1.562355 -1.0620633e-015 4.7831078 4.6870656 -1.0620633e-015 
		4.7831078 -4.6870656 -3.5402107e-016 1.5943692 -1.5623553 -3.5402107e-016 1.5943692 
		1.562355 -3.5402107e-016 1.5943692 4.6870656 -3.5402107e-016 1.5943692 -4.6870656 
		3.5402113e-016 -1.5943694 -1.5623553 3.5402113e-016 -1.5943694 1.562355 3.5402113e-016 
		-1.5943694 4.6870656 3.5402113e-016 -1.5943694 -4.6870656 1.0620633e-015 -4.7831078 
		-1.5623553 1.0620633e-015 -4.7831078 1.562355 1.0620633e-015 -4.7831078 4.6870656 
		1.0620633e-015 -4.7831078;
	setAttr -s 24 ".ed[0:23]"  0 1 0 0 4 0 
		1 2 0 1 5 1 2 3 0 2 6 1 
		3 7 0 4 5 1 4 8 0 5 6 1 
		5 9 1 6 7 1 6 10 1 7 11 0 
		8 9 1 8 12 0 9 10 1 9 13 1 
		10 11 1 10 14 1 11 15 0 12 13 0 
		13 14 0 14 15 0;
	setAttr -s 9 ".fc[0:8]" -type "polyFaces" 
		f 4 0 3 -8 -2 
		mu 0 4 0 1 5 4 
		f 4 2 5 -10 -4 
		mu 0 4 1 2 6 5 
		f 4 4 6 -12 -6 
		mu 0 4 2 3 7 6 
		f 4 7 10 -15 -9 
		mu 0 4 4 5 9 8 
		f 4 9 12 -17 -11 
		mu 0 4 5 6 10 9 
		f 4 11 13 -19 -13 
		mu 0 4 6 7 11 10 
		f 4 14 17 -22 -16 
		mu 0 4 8 9 13 12 
		f 4 16 19 -23 -18 
		mu 0 4 9 10 14 13 
		f 4 18 20 -24 -20 
		mu 0 4 10 11 15 14 ;
	setAttr ".cd" -type "dataPolyComponent" Index_Data Edge 0 ;
	setAttr ".cvd" -type "dataPolyComponent" Index_Data Vertex 0 ;
createNode joint -n "joint1";
	addAttr -ci true -sn "liw" -ln "lockInfluenceWeights" -min 0 -max 1 -at "bool";
	setAttr ".uoc" yes;
	setAttr ".t" -type "double3" 0.99799373473931041 0 5.6032220530578378 ;
	setAttr ".mnrl" -type "double3" -360 -360 -360 ;
	setAttr ".mxrl" -type "double3" 360 360 360 ;
	setAttr ".jo" -type "double3" 0 98.197657043460069 0 ;
	setAttr ".bps" -type "matrix" -0.14258845935391196 0 -0.98978206250622569 0 0 1 0 0
		 0.98978206250622569 0 -0.14258845935391196 0 0.99799373473931041 0 5.6032220530578378 1;
	setAttr ".radi" 0.73561220752988088;
createNode joint -n "joint2" -p "joint1";
	addAttr -ci true -sn "liw" -ln "lockInfluenceWeights" -min 0 -max 1 -at "bool";
	setAttr ".uoc" yes;
	setAttr ".oc" 1;
	setAttr ".t" -type "double3" 5.5551693455776965 0 -6.6613381477509392e-016 ;
	setAttr ".mnrl" -type "double3" -360 -360 -360 ;
	setAttr ".mxrl" -type "double3" 360 360 360 ;
	setAttr ".jo" -type "double3" 0 -10.497367444486464 0 ;
	setAttr ".bps" -type "matrix" 0.040126742148341554 0 -0.99919459794604604 0 0 1 0 0
		 0.99919459794604604 0 0.040126742148341554 0 0.2058906963033067 0 0.1048150806205852 1;
	setAttr ".radi" 0.76839582875359413;
createNode joint -n "joint3" -p "joint2";
	addAttr -ci true -sn "liw" -ln "lockInfluenceWeights" -min 0 -max 1 -at "bool";
	setAttr ".uoc" yes;
	setAttr ".oc" 2;
	setAttr ".t" -type "double3" 6.1889860225694884 0 -2.0816681711721685e-015 ;
	setAttr ".mnrl" -type "double3" -360 -360 -360 ;
	setAttr ".mxrl" -type "double3" 360 360 360 ;
	setAttr ".jo" -type "double3" 0 -87.700289598973598 0 ;
	setAttr ".bps" -type "matrix" 1 0 -6.9388939039072284e-018 0 0 1 0 0 6.9388939039072284e-018 0 1 0
		 0.45423454259064044 0 -6.0791863198944336 1;
	setAttr ".radi" 0.76839582875359413;
createNode lightLinker -s -n "lightLinker1";
	setAttr -s 2 ".lnk";
	setAttr -s 2 ".slnk";
createNode displayLayerManager -n "layerManager";
createNode displayLayer -n "defaultLayer";
createNode renderLayerManager -n "renderLayerManager";
createNode renderLayer -n "defaultRenderLayer";
	setAttr ".g" yes;
createNode skinCluster -n "skinCluster1";
	setAttr -s 16 ".wl";
	setAttr ".wl[0].w[0]"  1;
	setAttr -s 3 ".wl[1].w[0:2]"  0.99999999999999978 2.1309527186916592e-016 
		8.9493330558652603e-018;
	setAttr -s 3 ".wl[2].w[0:2]"  0.99999999999999978 2.1358488774986678e-016 
		8.4597171750987689e-018;
	setAttr ".wl[3].w[0]"  1;
	setAttr ".wl[4].w[0]"  1;
	setAttr ".wl[5].w[0]"  1;
	setAttr ".wl[6].w[0]"  1;
	setAttr ".wl[7].w[0]"  1;
	setAttr ".wl[8].w[1]"  1;
	setAttr ".wl[9].w[1]"  1;
	setAttr ".wl[10].w[1]"  1;
	setAttr ".wl[11].w[1]"  1;
	setAttr ".wl[12].w[1]"  1;
	setAttr ".wl[13].w[1]"  1;
	setAttr ".wl[14].w[1]"  1;
	setAttr ".wl[15].w[1]"  1;
	setAttr -s 3 ".pm";
	setAttr ".pm[0]" -type "matrix" -0.14258845935391193 0 0.98978206250622547 0 0 1 0 0
		 -0.98978206250622547 0 -0.14258845935391193 0 5.6882710694372891 0 -0.18884149717518511 1;
	setAttr ".pm[1]" -type "matrix" 0.040126742148341554 0 0.99919459794604604 0 0 1 0 0
		 -0.99919459794604604 0 0.040126742148341554 0 0.096468939458062744 0 -0.20993075922693388 1;
	setAttr ".pm[2]" -type "matrix" 1 0 6.9388939039072284e-018 0 0 1 0 0 -6.9388939039072284e-018 0 1 0
		 -0.4542345425906405 0 6.0791863198944336 1;
	setAttr ".gm" -type "matrix" 1 0 0 0 0 1 0 0 0 0 1 0 0 0 0 1;
	setAttr -s 3 ".ma";
	setAttr -s 3 ".dpf[0:2]"  4 4 4;
	setAttr -s 3 ".lw";
	setAttr -s 3 ".lw";
	setAttr ".mmi" yes;
	setAttr ".mi" 5;
	setAttr ".ucm" yes;
createNode tweak -n "tweak1";
createNode objectSet -n "skinCluster1Set";
	setAttr ".ihi" 0;
	setAttr ".vo" yes;
createNode groupId -n "skinCluster1GroupId";
	setAttr ".ihi" 0;
createNode groupParts -n "skinCluster1GroupParts";
	setAttr ".ihi" 0;
	setAttr ".ic" -type "componentList" 1 "vtx[*]";
createNode objectSet -n "tweakSet1";
	setAttr ".ihi" 0;
	setAttr ".vo" yes;
createNode groupId -n "groupId2";
	setAttr ".ihi" 0;
createNode groupParts -n "groupParts2";
	setAttr ".ihi" 0;
	setAttr ".ic" -type "componentList" 1 "vtx[*]";
createNode dagPose -n "bindPose1";
	setAttr -s 3 ".wm";
	setAttr -s 3 ".xm";
	setAttr ".xm[0]" -type "matrix" "xform" 1 1 1 0 0 0 0 0.99799373473931041 0
		 5.6032220530578378 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 0.75584008207884557 0 0.65475626787610375 1
		 1 1 yes;
	setAttr ".xm[1]" -type "matrix" "xform" 1 1 1 0 0 0 0 5.5551693455776965 0 -6.6613381477509392e-016 0
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 -0.091478741633527844 0 0.99580702941340304 1
		 1 1 yes;
	setAttr ".xm[2]" -type "matrix" "xform" 1 1 1 0 0 0 0 6.1889860225694884 0 -2.0816681711721685e-015 0
		 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 0 -0.69277458738454689 0 0.72115419368826439 1
		 1 1 yes;
	setAttr -s 3 ".m";
	setAttr -s 3 ".p";
	setAttr ".bp" yes;

createNode script -n "sceneConfigurationScriptNode";
	setAttr ".b" -type "string" "playbackOptions -min 1 -max 24 -ast 1 -aet 48 ";
	setAttr ".st" 6;
select -ne :time1;
	setAttr ".o" 1;
select -ne :renderPartition;
	setAttr -s 2 ".st";
select -ne :initialShadingGroup;
	setAttr ".ro" yes;
select -ne :initialParticleSE;
	setAttr ".ro" yes;
select -ne :defaultShaderList1;
	setAttr -s 2 ".s";
select -ne :postProcessList1;
	setAttr -s 2 ".p";
select -ne :renderGlobalsList1;
select -ne :hardwareRenderGlobals;
	setAttr ".ctrs" 256;
	setAttr ".btrs" 512;
select -ne :defaultHardwareRenderGlobals;
	setAttr ".fn" -type "string" "im";
	setAttr ".res" -type "string" "ntsc_4d 646 485 1.333";
connectAttr "skinCluster1GroupId.id" "testMeshShape.iog.og[0].gid";
connectAttr "skinCluster1Set.mwc" "testMeshShape.iog.og[0].gco";
connectAttr "groupId2.id" "testMeshShape.iog.og[1].gid";
connectAttr "tweakSet1.mwc" "testMeshShape.iog.og[1].gco";
connectAttr "skinCluster1.og[0]" "testMeshShape.i";
connectAttr "tweak1.vl[0].vt[0]" "testMeshShape.twl";
connectAttr "joint1.s" "joint2.is";
connectAttr "joint2.s" "joint3.is";
connectAttr "layerManager.dli[0]" "defaultLayer.id";
connectAttr "renderLayerManager.rlmi[0]" "defaultRenderLayer.rlid";
connectAttr "skinCluster1GroupParts.og" "skinCluster1.ip[0].ig";
connectAttr "skinCluster1GroupId.id" "skinCluster1.ip[0].gi";
connectAttr "bindPose1.msg" "skinCluster1.bp";
connectAttr "joint1.wm" "skinCluster1.ma[0]";
connectAttr "joint2.wm" "skinCluster1.ma[1]";
connectAttr "joint3.wm" "skinCluster1.ma[2]";
connectAttr "joint1.liw" "skinCluster1.lw[0]";
connectAttr "joint2.liw" "skinCluster1.lw[1]";
connectAttr "joint3.liw" "skinCluster1.lw[2]";
connectAttr "groupParts2.og" "tweak1.ip[0].ig";
connectAttr "groupId2.id" "tweak1.ip[0].gi";
connectAttr "skinCluster1GroupId.msg" "skinCluster1Set.gn" -na;
connectAttr "testMeshShape.iog.og[0]" "skinCluster1Set.dsm" -na;
connectAttr "skinCluster1.msg" "skinCluster1Set.ub[0]";
connectAttr "tweak1.og[0]" "skinCluster1GroupParts.ig";
connectAttr "skinCluster1GroupId.id" "skinCluster1GroupParts.gi";
connectAttr "groupId2.msg" "tweakSet1.gn" -na;
connectAttr "testMeshShape.iog.og[1]" "tweakSet1.dsm" -na;
connectAttr "tweak1.msg" "tweakSet1.ub[0]";
connectAttr "testMeshShapeOrig.w" "groupParts2.ig";
connectAttr "groupId2.id" "groupParts2.gi";
connectAttr "joint1.msg" "bindPose1.m[0]";
connectAttr "joint2.msg" "bindPose1.m[1]";
connectAttr "joint3.msg" "bindPose1.m[2]";
connectAttr "bindPose1.w" "bindPose1.p[0]";
connectAttr "bindPose1.m[0]" "bindPose1.p[1]";
connectAttr "bindPose1.m[1]" "bindPose1.p[2]";
connectAttr "joint1.bps" "bindPose1.wm[0]";
connectAttr "joint2.bps" "bindPose1.wm[1]";
connectAttr "joint3.bps" "bindPose1.wm[2]";
connectAttr "testMeshShape.iog" ":initialShadingGroup.dsm" -na;
// End of normalization.ma
