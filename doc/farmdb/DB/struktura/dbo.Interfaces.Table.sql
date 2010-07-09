/****** Object:  Table [dbo].[Interfaces]    Script Date: 03/18/2010 13:18:44 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [Interfaces](
	[InterfaceUID] [char](36) NOT NULL,
	[InterfaceParentInterfaceUID] [char](36) NULL,
	[InterfaceMachineUID] [char](36) NOT NULL,
	[InterfaceIP] [varchar](15) NULL,
	[InterfaceMAC] [char](17) NOT NULL,
	[InterfaceNetworkUID] [char](36) NOT NULL,
	[InterfaceDNSName] [varchar](30) NULL,
	[InterfaceSwitchNumber] [varchar](15) NULL,
	[InterfaceSwitchPosition] [varchar](4) NULL,
	[InterfaceNote] [ntext] NULL,
	[InterfacePreffered] [bit] NULL,
 CONSTRAINT [PK_Interfaces] PRIMARY KEY CLUSTERED 
(
	[InterfaceUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY],
 CONSTRAINT [IX_InterfacesUID] UNIQUE NONCLUSTERED 
(
	[InterfaceUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
ALTER TABLE [Interfaces]  WITH CHECK ADD  CONSTRAINT [FK_Interfaces_Machines] FOREIGN KEY([InterfaceMachineUID])
REFERENCES [Machines] ([MachineUID])
ON DELETE CASCADE
GO
ALTER TABLE [Interfaces] CHECK CONSTRAINT [FK_Interfaces_Machines]
GO
ALTER TABLE [Interfaces]  WITH CHECK ADD  CONSTRAINT [FK_Interfaces_Networks] FOREIGN KEY([InterfaceNetworkUID])
REFERENCES [Networks] ([NetworkUID])
GO
ALTER TABLE [Interfaces] CHECK CONSTRAINT [FK_Interfaces_Networks]
GO