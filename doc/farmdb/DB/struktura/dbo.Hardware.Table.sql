/****** Object:  Table [dbo].[Hardware]    Script Date: 03/18/2010 13:18:35 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [Hardware](
	[HardwareUID] [char](36) NOT NULL,
	[HardwareVendorUID] [char](36) NOT NULL CONSTRAINT [DF_Hardware_HardwareVendorUID]  DEFAULT ('3355d4bd-0ff0-488c-8c31-53dcdd5cd51d'),
	[HardwareVendorID] [varchar](20) NULL,
	[HardwareTypeDescription] [varchar](50) NOT NULL,
	[HardwareCPUDescription] [varchar](50) NULL,
	[HardwareCPUCount] [tinyint] NULL,
	[HardwareCPUCoreCount] [tinyint] NULL,
	[HardwareCPUHT] [bit] NULL,
	[HardwareCPUPerformance] [smallint] NULL,
	[HardwareRAMSize] [smallint] NULL,
	[HardwareHDDSize] [smallint] NULL,
	[HardwareHDDDescription] [varchar](50) NULL,
	[HardwareWeight] [tinyint] NULL,
	[HardwareHeight] [tinyint] NULL,
	[HardwareWidth] [tinyint] NULL,
	[HardwarePower] [smallint] NULL,
	[HardwareNote] [ntext] NULL,
	[HardwareImageName] [varchar](50) NULL,
 CONSTRAINT [PK_Hardware] PRIMARY KEY CLUSTERED 
(
	[HardwareUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY],
 CONSTRAINT [IX_HardwareUID] UNIQUE NONCLUSTERED 
(
	[HardwareUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
ALTER TABLE [Hardware]  WITH CHECK ADD  CONSTRAINT [FK_Vendors_Hardware] FOREIGN KEY([HardwareVendorUID])
REFERENCES [Vendors] ([VendorUID])
ON DELETE SET DEFAULT
GO
ALTER TABLE [Hardware] CHECK CONSTRAINT [FK_Vendors_Hardware]
GO
