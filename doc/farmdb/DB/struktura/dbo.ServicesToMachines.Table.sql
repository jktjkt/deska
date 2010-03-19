/****** Object:  Table [dbo].[ServicesToMachines]    Script Date: 03/18/2010 13:19:03 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [ServicesToMachines](
	[ServicesToMachinesMachineUID] [char](36) NOT NULL,
	[ServicesToMachinesServiceUID] [char](36) NOT NULL,
	[ServicesToMachinesServiceParameter] [varchar](50) NULL
) ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
ALTER TABLE [ServicesToMachines]  WITH CHECK ADD  CONSTRAINT [FK_FunctionsToMachines_Functions] FOREIGN KEY([ServicesToMachinesServiceUID])
REFERENCES [Services] ([ServiceUID])
ON DELETE CASCADE
GO
ALTER TABLE [ServicesToMachines] CHECK CONSTRAINT [FK_FunctionsToMachines_Functions]
GO
ALTER TABLE [ServicesToMachines]  WITH CHECK ADD  CONSTRAINT [FK_FunctionsToMachines_Machines] FOREIGN KEY([ServicesToMachinesMachineUID])
REFERENCES [Machines] ([MachineUID])
ON DELETE CASCADE
GO
ALTER TABLE [ServicesToMachines] CHECK CONSTRAINT [FK_FunctionsToMachines_Machines]
GO
