/****** Object:  Table [dbo].[Machines]    Script Date: 03/18/2010 13:18:53 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [Machines](
	[MachineUID] [char](36) NOT NULL,
	[MachineParentMachineUID] [char](36) NULL,
	[MachineSerialNumber] [varchar](20) NULL,
	[MachineWarrantyNumber] [varchar](20) NULL,
	[MachineInvNo] [varchar](50) NULL,
	[MachineHardwareUID] [char](36) NOT NULL CONSTRAINT [DF_Machines_MachineHardwareUID]  DEFAULT ('73bb0cc2-ade8-4ed3-8ba6-b485f452dbff'),
	[MachineCPUHT] [bit] NULL,
	[MachinePurchaseDate] [datetime] NULL,
	[MachineWarrantyEndDate] [datetime] NULL,
	[MachineKVMNumber] [tinyint] NULL,
	[MachineKVMPosition] [tinyint] NULL,
	[MachineRackNumber] [varchar](3) NULL,
	[MachineRackPosition] [tinyint] NULL,
	[MachineRackHPosition] [tinyint] NULL,
	[MachineSoftwareOS] [varchar](20) NULL,
	[MachineNote] [ntext] NULL,
	[MachineObsolete] [bit] NULL CONSTRAINT [DF_Machines_MachineObsolete]  DEFAULT ((0)),
 CONSTRAINT [PK_Machines] PRIMARY KEY CLUSTERED 
(
	[MachineUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY],
 CONSTRAINT [IX_MachinesUID] UNIQUE NONCLUSTERED 
(
	[MachineUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
EXEC sys.sp_addextendedproperty @name=N'MS_Description', @value=N'Hlavni tabulka stroju' , @level0type=N'SCHEMA',@level0name=N'dbo', @level1type=N'TABLE',@level1name=N'Machines'
GO
ALTER TABLE [Machines]  WITH CHECK ADD  CONSTRAINT [FK_Machines_Hardware] FOREIGN KEY([MachineHardwareUID])
REFERENCES [Hardware] ([HardwareUID])
ON DELETE SET DEFAULT
GO
ALTER TABLE [Machines] CHECK CONSTRAINT [FK_Machines_Hardware]
GO