/****** Object:  Table [dbo].[Networks]    Script Date: 03/18/2010 13:18:56 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [Networks](
	[NetworkUID] [char](36) NOT NULL,
	[NetworkName] [varchar](30) NULL,
	[NetworkIP] [varchar](50) NULL,
	[NetworkVLAN] [tinyint] NULL,
	[NetworkMask] [varchar](15) NULL,
	[NetworkNote] [ntext] NULL,
 CONSTRAINT [PK_Networks] PRIMARY KEY CLUSTERED 
(
	[NetworkUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO