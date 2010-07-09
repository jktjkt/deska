/****** Object:  Table [dbo].[DeletedItems]    Script Date: 03/18/2010 13:18:23 ******/
SET ANSI_NULLS ON
GO
SET QUOTED_IDENTIFIER ON
GO
SET ANSI_PADDING ON
GO
CREATE TABLE [DeletedItems](
	[DeletedItemUID] [char](36) NOT NULL,
	[DeletedItemUIDType] [varchar](30) NOT NULL,
	[DeletedItemDetails] [text] NULL,
	[DeletedItemDate] [datetime] NOT NULL,
 CONSTRAINT [PK_DeletedItems] PRIMARY KEY CLUSTERED 
(
	[DeletedItemUID] ASC
)WITH (PAD_INDEX  = OFF, STATISTICS_NORECOMPUTE  = OFF, IGNORE_DUP_KEY = OFF, ALLOW_ROW_LOCKS  = ON, ALLOW_PAGE_LOCKS  = ON) ON [PRIMARY]
) ON [PRIMARY] TEXTIMAGE_ON [PRIMARY]
GO
SET ANSI_PADDING OFF
GO