USE [FarmDBv2]
GO
/****** Object:  Table [dbo].[Faults]    Script Date: 11/29/2011 21:06:24 ******/
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'08b64941-76a0-4362-87ab-7cb58d2a2981', N'39cb0bfd-1fae-4628-9f46-f8a50dfee253', CAST(0x000099F5010C1A60 AS DateTime), NULL, N'Stroji pravdepodobne odeslo napajeni.', NULL, NULL, 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'0ebbccc0-b949-4148-bd2b-edad2109218f', N'e8a23846-dde6-4638-8181-52dfe1e37446', CAST(0x00009CD200B9F190 AS DateTime), NULL, N'Vadny disk nebo radic', N'Vadny disk nebo radic', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'1695acfb-1af4-45fc-b7c9-98740fbdfb16', N'd6b24ebe-57aa-43a2-89cd-0761a83e7beb', CAST(0x00009C25010C5750 AS DateTime), CAST(0x00009C2900CA2600 AS DateTime), N'Vadna pamet', N'Vadna pamet, kernel nenabootuje (cpu2 MCE, bank4 a adresa)', N'Vymenena pamet', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'175264c3-0d10-402c-bd9d-b4533c239e65', N'a69a3946-8b94-44e2-b3f9-7474b85d4df2', CAST(0x00009E7F00DE7920 AS DateTime), CAST(0x00009E7F00F31290 AS DateTime), N'Vadny zdroj', N'Vadny zdroj, stroj nelze spustit.', N'Vymena zdroje.', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'3cfdb456-6b95-4d63-af18-2a9537053f8b', N'e502f486-68f6-48ca-b5f9-3131cbac433c', CAST(0x00009C2400C5C100 AS DateTime), CAST(0x00009C4901121BE0 AS DateTime), N'Vadna deska', N'Vadna deska (nenabiha, nefunguje konzole v ILO)', N'Vymenena pamet (ac to zni nelogicky)', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'47604f6f-8908-48aa-ba24-c7fbf527abe0', N'de5bca84-2130-4a76-b419-a79e9722568a', CAST(0x00009E7F00DEBF70 AS DateTime), NULL, N'Rozpadly filesystem, chyby SCSI', N'Vadny disk nebo radic, podle chybovych hlasek spise disk.', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'48ce0866-eb46-48e4-967b-96d896ece1d5', N'faac9c91-b864-4ee4-816a-817f0186ae4b', CAST(0x00009C1B014AF690 AS DateTime), CAST(0x00009C1C01107600 AS DateTime), N'Vadny motherboard', N'Stroj se vypnul, klasicka zavada bl35p. 
28.5. - prvni vymena => opet vadna deska
1.6. - druha vymena => opet vadna deska
2.6. - treti pokus - opraveno. Zustala puvodni deska, pru stacilo cely server rozebrat a slozit, technik nevi, kde byl problem.', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'54b556da-218e-47d8-b0ad-cd17ac701aa7', N'e7682a27-1879-4939-9d86-76a5aeb21629', CAST(0x00009C380118F9B0 AS DateTime), CAST(0x00009C380118F9B0 AS DateTime), N'Vypnul se sam od sebe.', N'Vypnul se sam od sebe.', N'Spusten rucne.', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'630fad5c-f001-46b6-9c2b-143d2478a1d5', N'4fed5fa1-6f28-4420-bbf1-7a655882a588', CAST(0x00009C25010D2A40 AS DateTime), CAST(0x00009C25010D7090 AS DateTime), N'Vadny disk', N'Nefunkcni disk, netoci se, BIOS ho nevidi.', N'Vymena disku', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'643e4a7c-d4fb-423b-9fd0-4897f4c650a9', N'072fc6a0-bafa-477c-81f8-1510771ccab7', CAST(0x00009D9C00DCD340 AS DateTime), CAST(0x00009D9C00DD1990 AS DateTime), N'Vadny motherboard', N'Vadny motherboard', NULL, 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'6bacd087-e0c6-428a-8b13-dfd333c35e98', N'e8a23846-dde6-4638-8181-52dfe1e37446', CAST(0x00009C3801194000 AS DateTime), CAST(0x00009C3801194000 AS DateTime), N'DMA error', N'Vytuhl s touto hlaskou.', N'Restart', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'92efaa13-ab3e-4e40-8ec3-2107d5838aa2', N'df0b9eb2-5e0d-459f-a93e-b993cdad975e', CAST(0x00009D9C00DC8CF0 AS DateTime), CAST(0x00009D9C00DD1990 AS DateTime), N'Vadna pamet', N'Vadny pametovy modul ve slotu 5', NULL, 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'9932bb02-30a1-41a4-89e7-fbcdf1ba6a7f', N'feeadd41-93f4-4440-91b1-0928d1fd0ecb', CAST(0x00009C1B014B8330 AS DateTime), CAST(0x00009C2300BCB0B0 AS DateTime), N'Vadny motherboard', N'Stroj se vypnul, klasicka zavada bl35p. 
28.5. - prvni vymena => opet vadna deska
1.6. - druha vymena => opet vadna deska
2.6. - treti vymena => zadna zmena. Podle g107 se zda, ze problem neni v desce samoztne, ale nejakem kontaktu.
8.6. - ctvrta vymena => stroj nabehl, ale nenabehne kernel (MCE)
9.6. - vymenena pamet v bank4, stroj funguje', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'a27abb64-791c-402f-893b-2462cef6c2fe', N'40450248-7283-4455-85bc-8422fdd9f41d', CAST(0x00009C5D00E6FCD0 AS DateTime), NULL, N'vadna pamet', N'memory error threshold exceeded v IML -> vadna pamet (System memory, module 1)', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'a6ac55e4-5fbb-47f8-9d2b-e20a697d02e3', N'b30600a9-7b49-482c-ab9e-36b39ed51738', CAST(0x00009C2400C5C100 AS DateTime), CAST(0x00009C25010A6B20 AS DateTime), N'Vadna deska', N'Vadna deska (nenabiha, nefunguje konzole v ILO)', N'Vymena desky', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'b15d369b-5c89-4f66-bfff-1b685ef0ed69', N'be4fe012-fa9c-4e6d-a91c-363336a13795', CAST(0x00009D9C00DC46A0 AS DateTime), CAST(0x00009D9C00DC8CF0 AS DateTime), N'Vadny disk', N'Vadny disk', NULL, 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'b2a2a77b-c408-4ed1-9526-8ecdb349bf39', N'7c733883-1d21-4d5c-a797-037dc4508fdf', CAST(0x00009E7F00DE32D0 AS DateTime), CAST(0x00009E7F00F2CC40 AS DateTime), N'Vadny zdroj', N'Vadny zdroj, stroj nelze spustit', N'Vymena zdroje.', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'b7e54a28-bb2d-4512-a07f-a8aafd2c868d', N'e51f22d5-1154-4b65-8b4e-63d4885239ba', CAST(0x00009C2900CA6C50 AS DateTime), CAST(0x00009C3100C99960 AS DateTime), N'Vadny motherboard - vyrazeno', N'Stroj ma vadny motherboard nebo zdroj, stale se vypina. Byl vyrazen, zdroj (po vyzkouseni), disk a pameti jsou k dispozici.', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'c1a142be-fc2b-4bcc-901a-4d5362a002b1', N'b712493d-adf4-4b5e-8096-c58dfba888c4', CAST(0x00009C17009D6110 AS DateTime), CAST(0x00009C1700A17FC0 AS DateTime), N'test', N'test of a fault description', N'konec testu', 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'c794b1b3-6c78-4211-8107-d349d21aea2b', N'3fc8b604-94ae-4d20-90f3-88921a8ac049', CAST(0x00009CCB00FD8270 AS DateTime), CAST(0x00009CD4011102A0 AS DateTime), N'vadny eth0', N'Odesla sitova karta, obhospodarujici eth0. Stroj je zatim prepojen pres eth1.', N'Vymenena zakladni deska', 127)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'cb3c6538-4fff-496d-a267-70fa6fb7f49a', N'b1c9db8f-5820-4b93-b18d-6fd158f10d09', CAST(0x00009C25010C9DA0 AS DateTime), CAST(0x00009C25010D2A40 AS DateTime), N'Vadna deska', N'Stroj je vypnuty, pri pokusu o spusteni se pouze rozsviti cervena heartbeat LED.', N'Vymena desky.', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'd687b09d-471f-4a9d-bc9d-e48e0f6389ec', N'9706f12f-c590-4c56-b749-327c42476bb9', CAST(0x00009C25010BCAB0 AS DateTime), CAST(0x00009C25010C1100 AS DateTime), N'Vadna pamet', N'Vadna pamet, projevuje se pri bootu hlaskou cpu2 MCE, bank4 a adresa.', N'Vymena pameti', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'd8d7c3cb-ac9f-4577-825c-ca9bd122c5c4', N'd183bef0-ec9d-4f22-8d7d-e3ae44b9f526', CAST(0x00009C8E00B6A5D0 AS DateTime), NULL, N'server nenabiha', N'Server se vypnul a nelze spustit, sviti na nem cervena heartbeat led, v IML neni zadny zaznam. Nahlaseno na servis HP.', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'e9832e80-d2ba-4b47-adb5-dd7f14a2f55f', N'9c1df7a8-b98b-4e15-909c-301db3fedfd0', CAST(0x00009D9C00DCD340 AS DateTime), CAST(0x00009D9C00DD1990 AS DateTime), N'Vadna pamet', N'Vadny pametovy modul', NULL, 1)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'f268a7fe-6a1f-4766-87e8-70b9d0b76b1f', N'a2cfb265-8365-4e01-9817-da2decd3fe47', CAST(0x00009D3700D0BD80 AS DateTime), CAST(0x00009D3700D103D0 AS DateTime), N'Stroj se sam vypnul, uz podruhe', N'Stroj se sam vypnul, uz podruhe', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'f30eda54-08d0-447d-8e51-bd1d0d91b2ed', N'394e2e01-e211-4889-a72a-cd6255a897ef', CAST(0x00009C2400C5C100 AS DateTime), CAST(0x00009C4901126230 AS DateTime), N'Vadna deska', N'Vadna deska (nenabiha, nefunguje konzole v ILO)', N'Vymenena pamet (ac to zni nelogicky)', 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'f4a0801a-8897-447a-ab03-3c06d3232e92', N'b1c9db8f-5820-4b93-b18d-6fd158f10d09', CAST(0x00009CCD00E85C60 AS DateTime), NULL, N'Vadna pamet', N'Stroj nereaguje, log je plny ECC chyb', NULL, 255)
INSERT [dbo].[Faults] ([FaultUID], [FaultMachineUID], [FaultDate], [FaultSolvedDate], [FaultShort], [FaultDescription], [FaultSolution], [FaultSeverity]) VALUES (N'fc09ebfe-0f46-4a13-9484-d9f5953f25d4', N'c150a962-1a56-4b9f-8cc1-6c7d57229282', CAST(0x00009C2400C5C100 AS DateTime), NULL, N'Vadna deska', N'Vadna deska (nenabiha, nefunguje konzole v ILO)', NULL, 255)
