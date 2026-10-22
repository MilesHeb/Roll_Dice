<?xml version='1.0' encoding='UTF-8'?>
<Project Type="Project" LVVersion="26008000">
	<Property Name="NI.LV.All.SaveVersion" Type="Str">26.0</Property>
	<Property Name="NI.LV.All.SourceOnly" Type="Bool">true</Property>
	<Item Name="My Computer" Type="My Computer">
		<Property Name="server.app.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.control.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="server.tcp.enabled" Type="Bool">false</Property>
		<Property Name="server.tcp.port" Type="Int">0</Property>
		<Property Name="server.tcp.serviceName" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.tcp.serviceName.default" Type="Str">My Computer/VI Server</Property>
		<Property Name="server.vi.callsEnabled" Type="Bool">true</Property>
		<Property Name="server.vi.propertiesEnabled" Type="Bool">true</Property>
		<Property Name="specify.custom.address" Type="Bool">false</Property>
		<Item Name="Dice_face.ctl" Type="VI" URL="../SubVIs/Dice_face.ctl"/>
		<Item Name="Dice_Face_Pattern.vi" Type="VI" URL="../../../../../../LABView_Projects/Dice Roll/SubVI/Dice_Face_Pattern.vi"/>
		<Item Name="get_roll.vi" Type="VI" URL="../SubVIs/get_roll.vi"/>
		<Item Name="get_roll_serial_test.vi" Type="VI" URL="../SubVIs/get_roll_serial_test.vi"/>
		<Item Name="Graph_Parser.vi" Type="VI" URL="../../../../../../LABView_Projects/Dice Roll/SubVI/Graph_Parser.vi"/>
		<Item Name="local_roll.vi" Type="VI" URL="../SubVIs/local_roll.vi"/>
		<Item Name="Query.vi" Type="VI" URL="../SubVIs/Query.vi"/>
		<Item Name="Roll_Dice.vi" Type="VI" URL="../Controls/Roll_Dice.vi"/>
		<Item Name="Serial_Comm.vi" Type="VI" URL="../SubVIs/Serial_Comm.vi"/>
		<Item Name="Serial_test.vi" Type="VI" URL="../Serial_test.vi"/>
		<Item Name="Serial_test_Query.vi" Type="VI" URL="../Serial_test_Query.vi"/>
		<Item Name="Dependencies" Type="Dependencies"/>
		<Item Name="Build Specifications" Type="Build">
			<Item Name="Roll_Dice" Type="EXE">
				<Property Name="App_copyErrors" Type="Bool">true</Property>
				<Property Name="App_INI_aliasGUID" Type="Str">{1AF08ECD-FAC4-4644-9005-9541D8F67A60}</Property>
				<Property Name="App_INI_GUID" Type="Str">{8B56364D-27D3-4107-89D5-6D083B7FE99A}</Property>
				<Property Name="App_serverConfig.httpPort" Type="Int">8002</Property>
				<Property Name="App_serverType" Type="Int">0</Property>
				<Property Name="Bld_autoIncrement" Type="Bool">true</Property>
				<Property Name="Bld_buildCacheID" Type="Str">{97233DF3-8420-4A31-BEBE-F3B5B85BD1D2}</Property>
				<Property Name="Bld_buildSpecName" Type="Str">Roll_Dice</Property>
				<Property Name="Bld_excludeLibraryItems" Type="Bool">true</Property>
				<Property Name="Bld_excludePolymorphicVIs" Type="Bool">true</Property>
				<Property Name="Bld_excludeTypedefs" Type="Bool">true</Property>
				<Property Name="Bld_localDestDir" Type="Path">../builds/NI_AB_PROJECTNAME/Roll_Dice</Property>
				<Property Name="Bld_localDestDirType" Type="Str">relativeToCommon</Property>
				<Property Name="Bld_modifyLibraryFile" Type="Bool">true</Property>
				<Property Name="Bld_previewCacheID" Type="Str">{F759AC42-F3D0-4EA2-9FDC-180BCE7BB92E}</Property>
				<Property Name="Bld_version.build" Type="Int">1</Property>
				<Property Name="Bld_version.major" Type="Int">1</Property>
				<Property Name="Destination[0].destName" Type="Str">Roll Dice Example.exe</Property>
				<Property Name="Destination[0].path" Type="Path">../builds/NI_AB_PROJECTNAME/Roll_Dice/Roll Dice Example.exe</Property>
				<Property Name="Destination[0].preserveHierarchy" Type="Bool">true</Property>
				<Property Name="Destination[0].type" Type="Str">App</Property>
				<Property Name="Destination[1].destName" Type="Str">Support Directory</Property>
				<Property Name="Destination[1].path" Type="Path">../builds/NI_AB_PROJECTNAME/Roll_Dice/data</Property>
				<Property Name="DestinationCount" Type="Int">2</Property>
				<Property Name="Source[0].itemID" Type="Str">{9C86DD6A-FB85-4A95-833F-4F2C3B60618D}</Property>
				<Property Name="Source[0].type" Type="Str">Container</Property>
				<Property Name="Source[1].destinationIndex" Type="Int">0</Property>
				<Property Name="Source[1].itemID" Type="Ref">/My Computer/Roll_Dice.vi</Property>
				<Property Name="Source[1].sourceInclusion" Type="Str">TopLevel</Property>
				<Property Name="Source[1].type" Type="Str">VI</Property>
				<Property Name="SourceCount" Type="Int">2</Property>
				<Property Name="TgtF_companyName" Type="Str">Miles Hebestadt</Property>
				<Property Name="TgtF_fileDescription" Type="Str">Roll_Dice</Property>
				<Property Name="TgtF_internalName" Type="Str">Roll_Dice</Property>
				<Property Name="TgtF_legalCopyright" Type="Str">Copyright © 2026 Miles Hebestadt</Property>
				<Property Name="TgtF_productName" Type="Str">Roll_Dice</Property>
				<Property Name="TgtF_targetfileGUID" Type="Str">{680AA55F-898E-4882-9814-FDB7F16C1D3D}</Property>
				<Property Name="TgtF_targetfileName" Type="Str">Roll Dice Example.exe</Property>
				<Property Name="TgtF_versionIndependent" Type="Bool">true</Property>
			</Item>
		</Item>
	</Item>
</Project>
