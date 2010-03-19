/****** Object:  Table [dbo].[Racks]    Script Date: 03/18/2010 13:19:00 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [Racks](
	[RackUID] [char](36) NOT NULL,
	[RackInnerWidth] [tinyint] NULL,
	[RackInnerHeight] [tinyint] NULL,
	[RackOuterWidth] [tinyint] NULL,
	[RackOuterHeight] [tinyint] NULL,
	[RackName] [varchar](50) NOT NULL,
	[RackNote] [ntext] NULL,
 CONSTRAINT [PK_Racks] PRIMARY KEY CLUSTERED 
(
	[RackUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO
