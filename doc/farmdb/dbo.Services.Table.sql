USE [FarmDBv2]
GO
/****** Object:  Table [dbo].[Services]    Script Date: 11/29/2011 21:06:24 ******/
INSERT [dbo].[Services] ([ServiceUID], [ServiceParentServiceUID], [ServiceName]) VALUES (N'0fc8ac64-9b68-4fd7-817a-f6f1efa22c54', N'75112bac-bbcd-4e56-9b6e-f8b86a2bf186', N'router')
INSERT [dbo].[Services] ([ServiceUID], [ServiceParentServiceUID], [ServiceName]) VALUES (N'2aa7a20d-dec8-4ee3-952a-7702f132a9f9', NULL, N'workernode')
INSERT [dbo].[Services] ([ServiceUID], [ServiceParentServiceUID], [ServiceName]) VALUES (N'5f698886-37a2-4f0a-a0b7-7573a8228674', N'75112bac-bbcd-4e56-9b6e-f8b86a2bf186', N'dns server')
INSERT [dbo].[Services] ([ServiceUID], [ServiceParentServiceUID], [ServiceName]) VALUES (N'75112bac-bbcd-4e56-9b6e-f8b86a2bf186', NULL, N'network')
INSERT [dbo].[Services] ([ServiceUID], [ServiceParentServiceUID], [ServiceName]) VALUES (N'7b52b06c-3eff-4803-a72b-0590b7cafef1', N'75112bac-bbcd-4e56-9b6e-f8b86a2bf186', N'dhcp server')
