#include "APILIB.h"
#include <algorithm>
#include <iterator>

CompareCell CreateCmpCell(std::string column_name, Column_Type column_type, Operator_Type Optype, std::string value)
{

	Column_Cell column_cell;
	column_cell.columu_name = column_name;
	Column_Type tmp = column_type;
	char*pChar = nullptr;
	switch (tmp)
	{
	case Column_Type::I:
		column_cell.column_type = Column_Type::I;
		column_cell.column_value.IntValue = stoi(value);
		break;

	case Column_Type::C:
		column_cell.column_type = Column_Type::C;
		pChar = (char*)malloc(value.size() + 1);
		strcpy(pChar, value.c_str());
		column_cell.column_value.StrValue = pChar;
		break;

	case Column_Type::D:
		column_cell.column_type = Column_Type::D;
		column_cell.column_value.IntValue = stod(value);
		break;
	default:
		break;
	}
	CompareCell cmp_cell(Optype, column_cell);

	return cmp_cell;
}

bool CreateDatabase(std::string db_name, CatalogPosition &cp)
{
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		tmp_path = cp.GetRootPath() + db_name;
		_mkdir(tmp_path.c_str());
		return true;
	}
	else
	{
		std::cout << "数据库已经存在" << std::endl;
		return false;
	}
}

bool DropDatabase(std::string db_name, CatalogPosition &cp)
{
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		std::cout << "数据库不存在" << std::endl;
		return false;
	}
	else
	{
		tmp_path = cp.GetRootPath() + db_name;
		// 删除目录下文件
		auto t = tmp_path + "/";
		DelFilesInFolder(t);
		// 删除目录
		_rmdir(tmp_path.c_str());

		return true;
	}

	return false;
}

void DelFilesInFolder(std::string folderPath)
{
	_finddata_t FileInfo;
	std::string strfind = folderPath + "*";
	long Handle = _findfirst(strfind.c_str(), &FileInfo);

	if (Handle == -1L)
	{
		std::cerr << "can not match the folder path" << std::endl;
		return;
	}
	do {
		//判断是否有子目录  
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//这个语句很重要  
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				std::string newPath = folderPath + FileInfo.name;
				newPath += "/";
				DelFilesInFolder(newPath);
				// 删除该文件夹
				_rmdir(newPath.c_str());
			}
		}
		else
		{
			//fout << folderPath << FileInfo.name << " ";
			// 删除文件
			auto file = folderPath + FileInfo.name;
			remove(file.c_str());
			std::cout << std::endl;
		}
	} while (_findnext(Handle, &FileInfo) == 0);

	_findclose(Handle);
}

std::vector<std::string> ShowDatabase(CatalogPosition &cp)
{
	_finddata_t FileInfo;
	std::string path = cp.GetRootPath() + "*.*";
	int k;
	long HANDLE;
	k = HANDLE = _findfirst(path.c_str(), &FileInfo);
	std::vector<std::string> dbs;

	while (k != -1)
	{
		// 如果是普通文件夹则输出
		if (FileInfo.attrib&_A_SUBDIR && strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
		{
			dbs.push_back(FileInfo.name);
			//std::cout << FileInfo.name << std::endl;
		}

		k = _findnext(HANDLE, &FileInfo);
	}
	_findclose(HANDLE);

	return dbs;
}

bool UseDatabase(std::string db_name, CatalogPosition &cp)
{
	// 先判断数据库是否存在
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		return false;
	}
	else
	{
		cp.SetCurrentPath(cp.GetRootPath() + db_name + "/");
		cp.isInSpeDb = true;
		return true;
	}
}

bool CreateTable(TB_Create_Info tb_create_info, std::string path)
{
	// TODO 检查创建信息以及当前目录是否在数据库中

	// 获取表名
	std::string table_name = tb_create_info.table_name;

	// 获取关键字位置
	int KeyTypeIndex = 0;
	for (int j = 0; j < tb_create_info.columns_info.size(); j++)
	{
		if (tb_create_info.columns_info[j].isPrimary)
		{
			KeyTypeIndex = j;
			break;
		}
	}

	//获取字段信息
	char RecordTypeInfo[RecordColumnCount];          // 记录字段类型信息
	char *ptype = RecordTypeInfo;
	char RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
	char*pname = RecordColumnName;

	auto &column_info_ref = tb_create_info.columns_info;
	for (int i = 0; i < column_info_ref.size(); i++)
	{
		// 类型信息
		switch (column_info_ref[i].type)
		{
		case Column_Type::I:
			*ptype++ = 'I';
			break;

		case  Column_Type::D:
			*ptype++ = 'D';
			break;

		case Column_Type::C:
			*ptype++ = 'C';
			*ptype++ = column_info_ref[i].length / 100 + '0';
			*ptype++ = (column_info_ref[i].length % 100) / 10 + '0';
			*ptype++ = column_info_ref[i].length % 10 + '0';
		default:
			break;
		}
		// 名称信息
		strcpy(pname, column_info_ref[i].name.c_str());
		pname += ColumnNameLength;
	}
	*ptype = '\0';

	// 创建索引文件
	std::string idx_file = path + table_name + ".idx";
	BTree tree(idx_file, KeyTypeIndex, RecordTypeInfo, RecordColumnName);
	// 创建数据文件
	std::string dbf_file = path + table_name + ".dbf";
	GetGlobalFileBuffer().CreateFile(dbf_file.c_str());
	return true;
}

bool DropTable(std::string table_name, std::string path /*= std::string("./")*/)
{
	std::string tmp_path = path + table_name;
	std::string idx = tmp_path + ".idx";
	std::string dbf = tmp_path + ".dbf";

	if (!GetCp().GetIsInSpeDb())
		return false;

	if (_access(idx.c_str(), 0) == -1|| _access(dbf.c_str(), 0) == -1)  //判断表是否存在
	{
		return false;
	}
	else
	{
		// 删除表文件
		remove(idx.c_str());
		remove(dbf.c_str());
		return true;
	}

	return false;
}

std::vector<std::string> ShowAllTable(bool b, std::string path /*= std::string("./")*/)
{
	std::vector<std::string> dbs;
	if (!b)
		return dbs;

	_finddata_t FileInfo;
	path += "*.*";
	int k;
	long HANDLE;
	k = HANDLE = _findfirst(path.c_str(), &FileInfo);
	

	while (k != -1)
	{
		// 如果是普通文件夹则输出
		if (!(FileInfo.attrib&_A_SUBDIR) && strcmp(FileInfo.name, ".") != 0 && strcmp(FileInfo.name, "..") != 0)
		{
			dbs.push_back(FileInfo.name);
		}

		k = _findnext(HANDLE, &FileInfo);
	}
	_findclose(HANDLE);

	return dbs;
}

bool InsertRecord(TB_Insert_Info tb_insert_info, std::string path /*= std::string("./")*/)
{
	if (!GetCp().GetIsInSpeDb())  // 如果不在具体数据库目录下，则不能插入记录
		return false;

	std::string idx_file = path + tb_insert_info.table_name + ".idx";
	std::string dbf_file = path + tb_insert_info.table_name + ".dbf";
	BTree tree(idx_file);
	auto phead = tree.GetPtrIndexHeadNode();

	KeyAttr key;

#ifndef NDEBUG
	std::cout << "打印索引头部信息" << std::endl;
	std::cout << "记录字段类型：" << phead->RecordTypeInfo << std::endl;
#endif

	int sz_col = 0;// 字段个数
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I' || phead->RecordTypeInfo[i] == 'C' || phead->RecordTypeInfo[i] == 'D')sz_col++;
	}

#ifndef NDEBUG
	std::cout << "各个字段名称：" << std::endl;
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		std::cout << pColumnName << std::endl;
		pColumnName += ColumnNameLength;
	}
#endif

	// 将记录信息封装成记录数据对象
	RecordHead record_head;
	int column_id = 0;
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		Column_Cell cc;
		if (phead->RecordTypeInfo[i] == 'I')
		{
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{

				cc.column_type = Column_Type::I;
				cc.column_value.IntValue = stoi(tb_insert_info.insert_info[k].column_value);
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::I;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'D')
		{
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{

				cc.column_type = Column_Type::D;
				cc.column_value.DoubleValue = stod(tb_insert_info.insert_info[k].column_value);
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::D;
			}

			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'C')
		{
			//找到对应的字段名称
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//在插入记录里寻找该字段的值
			int k = -1;
			for (int j = 0; j < tb_insert_info.insert_info.size(); j++)
			{
				if (pColumnName == tb_insert_info.insert_info[j].column_name)
				{
					k = j;
					break;
				}
			}

			if (k != -1)
			{
				cc.column_type = Column_Type::C;
				int sz = (phead->RecordTypeInfo[i + 1] - '0') * 100 + (phead->RecordTypeInfo[i + 2] - '0') * 10 + (phead->RecordTypeInfo[i + 3] - '0');
				char*pChar = (char*)malloc(sz + 3); // 多申请三个字节用来保存用户定义的字符串长度值
				memcpy(pChar, &(phead->RecordTypeInfo[i + 1]), 3);
				pChar += 3;
				strcpy(pChar, tb_insert_info.insert_info[k].column_value.c_str());
				cc.column_value.StrValue = pChar - 3;
			}
			else
			{
				// 默认值填充
				cc.column_type = Column_Type::C;
				cc.column_value.StrValue = (char*)malloc(4);
				cc.column_value.StrValue[0] = cc.column_value.StrValue[1] = cc.column_value.StrValue[2] = '0';
				cc.column_value.StrValue[3] = '\0';
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

	}

	// 插入数据文件
	Record record;
	auto fd = record.InsertRecord(dbf_file, record_head);
	// 插入索引
	int key_index = 0;
	auto p = record_head.GetFirstColumn();
	while (key_index != phead->KeyTypeIndex)
	{
		p = p->next;
		key_index++;
	}
	key = *p;

	tree.Insert(key, fd);
	return true;
}

SelectPrintInfo SelectTable(TB_Select_Info tb_select_info, std::string path)
{
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	GetTimer().Start();
	if (tb_select_info.vec_cmp_cell.empty())  // 查找所有记录
	{
		// 索引文件名
		std::string file_idx = path + tb_select_info.table_name + ".idx";

		// 读取索引 信息
		BTree tree(file_idx);
		auto phead = tree.GetPtrIndexHeadNode();

		// 第一个数据结点地址
		auto node_fd = phead->MostLeftNode;

		while (node_fd.offSet != 0)
		{
			//取出结点内记录
			auto pNode = tree.FileAddrToMemPtr(node_fd);
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				res.push_back({ pNode->key[i], pNode->children[i] });
			}
			// 下一个数据结点
			node_fd = tree.FileAddrToMemPtr(node_fd)->next;
		}
	}
	else
	{
		for (int i = 0; i < tb_select_info.vec_cmp_cell.size(); i++)
		{
			// 查找满足单个字段的记录
			fds = Search(tb_select_info.vec_cmp_cell[i], tb_select_info.table_name, GetCp().GetCurrentPath());
			// 新的结果和之前的结果求交集
			if (res.empty())
			{
				res = fds;
			}
			else
			{
				std::vector<std::pair<KeyAttr, FileAddr>> v;
				sort(fds.begin(), fds.end());
				sort(res.begin(), res.end());
				set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
				res = v;
			}

		}
	}
	GetTimer().Stop();
	SelectPrintInfo spi;
	spi.table_name = tb_select_info.table_name;
	spi.name_selected_column = tb_select_info.name_selected_column;
	spi.key_fd = res;
	return spi;
}

bool UpdateTable(TB_Update_Info tb_update_info, std::string path /*= std::string("./")*/)
{
	std::string file_idx = path + tb_update_info.table_name + ".idx";
	std::string file_dbf = path + tb_update_info.table_name + ".dbf";
	BTree tree(file_idx);
	// 读取对应字段的长度
	std::vector<std::string> col_name;
	std::vector<int> col_len;
	auto phead = tree.GetPtrIndexHeadNode();
	int sz_col = 0; // 字段个数
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I')
		{
			col_len.push_back(sizeof(int));
			sz_col++;
		}
		if (phead->RecordTypeInfo[i] == 'D')
		{
			col_len.push_back(sizeof(double));
			sz_col++;
		}
		if (phead->RecordTypeInfo[i] == 'C')
		{
			int len = 0;
			for (int j = 1; j <= 3; j++)
			{
				len += ((phead->RecordTypeInfo[i + j] - '0') * 10);
			}
			col_len.push_back(len);
			sz_col++;
		}
		
	}
	// 各个字段名称
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		col_name.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}

	// 将更新操作的expr条件封装成查找条件
	std::vector<CompareCell> cmp_cells;
	auto fields_name = GetColumnAndTypeFromTable(tb_update_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_update_info.expr.size(); i++)
	{
		CompareCell cmp_cell = CreateCmpCell(tb_update_info.expr[i].field, GetType(tb_update_info.expr[i].field, fields_name)
			, GetOperatorType(tb_update_info.expr[i].op), tb_update_info.expr[i].value);
		
		cmp_cells.push_back(cmp_cell);
	}
	
	// 查找满足更新条件的字段
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	// 否则
	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// 查找满足单个字段的记录
		fds = Search(cmp_cells[i], tb_update_info.table_name, GetCp().GetCurrentPath());
		// 新的结果和之前的结果求交集
		if (res.empty())
		{
			res = fds;
		}
		else
		{
			std::vector<std::pair<KeyAttr, FileAddr>> v;
			sort(fds.begin(), fds.end());
			sort(res.begin(), res.end());
			set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
			res = v;
		}

	}

	// 更新记录
	for (int i = 0; i < res.size(); i++)
	{
		// 先读出旧的记录
		auto record_head = GetDbfRecord(tb_update_info.table_name, res[i].second, path);
		//更该每个要更新的字段值
		
		for (int j = 0; j < tb_update_info.field_value.size(); j++)
		{
			auto head = record_head.GetFirstColumn();
			std::string new_str;
			int n = 0;
			while (head)
			{
				if (head->columu_name == tb_update_info.field_value[j].field)
				{
					// 更新
					switch (head->column_type)
					{
					case Column_Type::I:
						head->column_value.IntValue = stoi(tb_update_info.field_value[i].value);
						break;
					case Column_Type::D:
						head->column_value.DoubleValue = stod(tb_update_info.field_value[i].value);
						break;
					case Column_Type::C:
						// 获取长度
						for (int kkk = 0; kkk < col_name.size(); kkk++)
						{
							if (col_name[kkk] == head->columu_name)
							{
								n = kkk;
								break;
							}
						}
						new_str += IntToStr3(col_len[n]);
						new_str += tb_update_info.field_value[i].value;
						memcpy(head->column_value.StrValue, new_str.c_str(), new_str.size()+1);
						//strcpy(head->column_value.StrValue, new_str.c_str());
						break;
					default:
						break;
					}
					
					break;
				}
				head = head->next;
			}
		}

		//写回
		Record record;
		record.UpdateRecord(file_dbf, record_head, res[i].second);
		
		// 判断主键的值有没有被更改
		bool isPrimary = false;  
		std::string new_value_primary;
		
		auto pKey_name = tree.GetPtrIndexHeadNode()->RecordColumnName;
		pKey_name += ColumnNameLength * tree.GetPtrIndexHeadNode()->KeyTypeIndex;
		for (int j = 0; j < tb_update_info.field_value.size(); j++)
		{
			if (tb_update_info.field_value[j].field == pKey_name)
			{
				isPrimary = true;
				new_value_primary = tb_update_info.field_value[j].value;
				break;
			}
		}
		// 修改主键索引
		if (isPrimary)
		{
			// TODO
			Column_Type old_key_type = res[i].first.type;
			tree.Delete(res[i].first);
			KeyAttr new_key;
			new_key.type = old_key_type;
			switch (new_key.type)
			{
			case Column_Type::I:
				new_key.value.IntValue = stoi(new_value_primary);
				break;
			case Column_Type::D:
				new_key.value.IntValue = stod(new_value_primary);
				break;
			case Column_Type::C:
				strcpy(new_key.value.StrValue, new_value_primary.c_str());
				break;
			default:
				break;
			}
			tree.Insert(new_key, res[i].second);
		}
	}
	return true;
}

bool DeleteTable(TB_Delete_Info tb_delete_info, std::string path /*= std::string("./")*/)
{
	std::string file_idx = path + tb_delete_info.table_name + ".idx";
	std::string file_dbf = path + tb_delete_info.table_name + ".dbf";

	// 将更新操作的expr条件封装成查找条件
	std::vector<CompareCell> cmp_cells;
	auto fields_name = GetColumnAndTypeFromTable(tb_delete_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_delete_info.expr.size(); i++)
	{
		CompareCell cmp_cell = CreateCmpCell(tb_delete_info.expr[i].field, GetType(tb_delete_info.expr[i].field, fields_name)
			, GetOperatorType(tb_delete_info.expr[i].op), tb_delete_info.expr[i].value);
		cmp_cells.push_back(cmp_cell);
	}

	// 查找满足更新条件的字段
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	// 否则
	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// 查找满足单个字段的记录
		fds = Search(cmp_cells[i], tb_delete_info.table_name, GetCp().GetCurrentPath());
		// 新的结果和之前的结果求交集
		if (res.empty())
		{
			res = fds;
		}
		else
		{
			std::vector<std::pair<KeyAttr, FileAddr>> v;
			sort(fds.begin(), fds.end());
			sort(res.begin(), res.end());
			set_intersection(fds.begin(), fds.end(), res.begin(), res.end(), std::back_inserter(v));
			res = v;
		}

	}

	// 删除所有删除的结果
	BTree tree(file_idx);
	Record record;
	for (int i = 0; i < res.size(); i++)
	{
		tree.Delete(res[i].first);
		record.DeleteRecord(file_dbf, res[i].second, 0);
	}
	
	return true;
}

std::vector<RecordHead> ShowTable(std::string table_name, std::string path /*= std::string("./")*/)
{
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";
	BTree tree(idx_file);
	std::vector<RecordHead> vec_record_head;

	auto data_fd = tree.GetPtrIndexHeadNode()->MostLeftNode;
	while (data_fd.offSet != 0)
	{
		auto pNode = tree.FileAddrToMemPtr(data_fd);

		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			auto tmp = GetDbfRecord(table_name, pNode->children[i], path);
			vec_record_head.push_back(tmp);
		}


		data_fd = pNode->next;
	}

	return vec_record_head;
}

RecordHead GetDbfRecord(std::string table_name, FileAddr fd, std::string path /*= std::string("./")*/)
{
	std::string idx_file = path + table_name + ".idx";
	std::string dbf_file = path + table_name + ".dbf";
	BTree tree(idx_file);

	RecordHead record_head;
	// 获取结点内存地址
	char* pRecTypeInfo = tree.GetPtrIndexHeadNode()->RecordTypeInfo;
	//std::cout << pRecTypeInfo << std::endl;
	auto pdata = (char*)GetGlobalFileBuffer()[dbf_file.c_str()]->ReadRecord(&fd);
	pdata += sizeof(FileAddr);  // 每条记录头部默认添加该记录的地址值

	auto vec_name_type = GetColumnAndTypeFromTable(table_name, path);
	int index = 0;
	while (*pRecTypeInfo != '\0')
	{
		Column_Cell cc;
		switch (*pRecTypeInfo)
		{
		case 'I':
			cc.column_type = Column_Type::I;
			cc.columu_name = vec_name_type[index].first;
			cc.column_value.IntValue = *(int*)pdata;
			pdata += sizeof(int);
			record_head.AddColumnCell(cc);
			index++;
			break;

		case 'D':
			cc.column_type = Column_Type::D;
			cc.columu_name = vec_name_type[index].first;
			cc.column_value.DoubleValue = *(double*)pdata;
			pdata += sizeof(double);
			record_head.AddColumnCell(cc);
			index++;
			break;

		case 'C':
			cc.column_type = Column_Type::C;
			cc.columu_name = vec_name_type[index].first;
			// 读取字符串长度
			int sz = 0;
			sz = (*(pRecTypeInfo + 1) - '0') * 100 + (*(pRecTypeInfo + 2) - '0') * 10 + (*(pRecTypeInfo + 3) - '0');
			auto pchar = (char*)malloc(sz);
			memcpy(pchar, pdata, sz);
			cc.column_value.StrValue = pchar;
			pdata += sz;
			record_head.AddColumnCell(cc);
			index++;
			break;
		}
		pRecTypeInfo++;
	}

	return record_head;
}


Operator_Type GetOperatorType(std::string s)
{
	s = StrToLower(s);
	if (s == ">")
	{
		return Operator_Type::B;
	}
	else if (s == ">=")
	{
		return Operator_Type::BE;
	}
	else if (s == "<")
	{
		return Operator_Type::L;
	}
	else if (s == "<=")
	{
		return Operator_Type::LE;
	}
	else if (s == "=")
	{
		return Operator_Type::E;
	}
	else if (s == "!=")
	{
		return Operator_Type::NE;
	}
	else
	{
		return Operator_Type::B;
	}
}

std::vector<std::pair<std::string, Column_Type>> GetColumnAndTypeFromTable(std::string table_name, std::string table_path)
{
	std::string idx_file = table_path + table_name + ".idx";
	std::string dbf_file = table_path + table_name + ".dbf";
	BTree tree(idx_file);

	auto phead = tree.GetPtrIndexHeadNode();

	// 记录各个字段类型
	std::vector<Column_Type> tb_types;
	int sz_col = 0;// 字段个数
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I')
		{
			tb_types.push_back(Column_Type::I);
			sz_col++;
		}
		else if (phead->RecordTypeInfo[i] == 'D')
		{
			tb_types.push_back(Column_Type::D);
			sz_col++;
		}
		else if (phead->RecordTypeInfo[i] == 'C')
		{
			tb_types.push_back(Column_Type::C);
			sz_col++;
		}

	}

	// 记录各个字段名称
	std::vector<std::string> tb_names;
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		tb_names.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}

	std::vector<std::pair<std::string, Column_Type>> res;
	for (int i = 0; i < tb_names.size(); i++)
	{
		res.push_back({ tb_names[i], tb_types[i] });
	}
	return res;
}


Column_Type GetType(std::string name, std::vector<std::pair<std::string, Column_Type>> vec)
{
	for (int i = 0; i < vec.size(); i++)
	{
		if (vec[i].first == name)
		{
			return vec[i].second;
		}
	}

	return Column_Type::I;
}

std::vector<std::pair<KeyAttr, FileAddr>> Search(CompareCell compare_cell, std::string table_name, std::string path /*= std::string("./")*/)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 读取索引 信息
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// 判断待查找的字段是否是主键字段
	bool bKeyComumn = false;
	int sz_col = 0;// 字段个数
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I' || phead->RecordTypeInfo[i] == 'C' || phead->RecordTypeInfo[i] == 'D')
			sz_col++;
	}

	int key_index = phead->KeyTypeIndex;
	char *pColumnName = phead->RecordColumnName;
	for(int i=0; i<key_index;i++)
		pColumnName += ColumnNameLength;


	if (compare_cell.cmp_value.columu_name == pColumnName)
	{
		bKeyComumn = true;
	}
		
	// 查找

	if (bKeyComumn)
	{
#ifndef NDEBUG
		std::cout << "主键查找" << std::endl;
#endif
		res = KeySearch(compare_cell, table_name, path);
	}
	else
	{
#ifndef NDEBUG
		std::cout << "遍历查找" << std::endl;
#endif
		res = RangeSearch(compare_cell, table_name, path);
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> KeySearch(CompareCell compare_cell, std::string table_name, std::string path /*= std::string("./")*/)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 读取索引 信息
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// 如果是查找相等的值
	if (compare_cell.OperType == Operator_Type::E)
	{
#ifndef NDEBUG
		std::cout << "主键B+树查找" << std::endl;
#endif
		auto fd = tree.Search(compare_cell.cmp_value);
		if (fd.offSet!=0)
		{
			res.push_back({ compare_cell.cmp_value , fd });
		}
	}
	else  // 可优化::关键字二分查找
	{
#ifndef NDEBUG
		std::cout << "主键索引遍历查找" << std::endl;
#endif
		// 第一个数据结点地址
		auto node_fd = phead->MostLeftNode;

		while (node_fd.offSet != 0)
		{
			//取出结点内记录
			auto pNode = tree.FileAddrToMemPtr(node_fd);
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				// 查找比较的字段
				Column_Cell cc(pNode->key[i]);
				bool isSearched = compare_cell(cc);
				if (isSearched)  // 满足条件
				{
					res.push_back({ pNode->key[i] ,pNode->children[i] });
				}
			}
			// 下一个数据结点
			node_fd = tree.FileAddrToMemPtr(node_fd)->next;
		}
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> RangeSearch(CompareCell compare_cell, std::string table_name, std::string path)
{
	// 保存查找结果
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// 索引文件名
	std::string file_idx = path + table_name + ".idx";

	// 读取索引 信息
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// 第一个数据结点地址
	auto node_fd = phead->MostLeftNode;

	while (node_fd.offSet!=0)
	{
		//取出结点内记录
		auto pNode = tree.FileAddrToMemPtr(node_fd);
		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			RecordHead record = GetDbfRecord(table_name, pNode->children[i], path);

			// 查找比较的字段
			auto pColumn = record.GetFirstColumn();
			while (pColumn && pColumn->columu_name != compare_cell.cmp_value.columu_name)pColumn = pColumn->next;
			bool isSearched = compare_cell(*pColumn);
			if (isSearched)  // 满足条件
			{
				res.push_back({ pNode->key[i] ,pNode->children[i] });
			}
		}
		// 下一个数据结点
		node_fd = tree.FileAddrToMemPtr(node_fd)->next;
	}

	return res;
}


CatalogPosition& GetCp()
{
	static CatalogPosition cp;
	return cp;
}

bool CatalogPosition::isInSpeDb = false;

CatalogPosition::CatalogPosition()
	:root("./DB/"), current_catalog("./DB/")
{
	// 如果当前目录下没有 DB 文件见则创建
	std::string tmp_path = "./DB";

	if (_access(tmp_path.c_str(), 0) == -1)
	{
		_mkdir(tmp_path.c_str());
	}
}

bool CatalogPosition::ResetRootCatalog(std::string root_new)
{
	if (root_new[root_new.size() - 1] == '/')
	{
		root = root_new;
		current_catalog = root;
		isInSpeDb = false;
		return true;
	}
	else
	{
		return false;
	}
}

void CatalogPosition::SwitchToDatabase()
{
	current_catalog = root;
	isInSpeDb = false;
}

bool CatalogPosition::SwitchToDatabase(std::string db_name)
{
	std::string tmp_path = root + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //判断数据库是否存在
	{
		return false;
	}
	else
	{
		current_catalog = root + db_name + "/";
		isInSpeDb = true;
		return true;
	}

}

std::string CatalogPosition::GetCurrentPath() const
{
	return current_catalog;
}

std::string CatalogPosition::GetRootPath() const
{
	return root;
}

std::string CatalogPosition::SetCurrentPath(std::string cur)
{
	current_catalog = cur;
	return current_catalog;
}
