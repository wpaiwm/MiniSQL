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

	if (_access(tmp_path.c_str(), 0) == -1)  //�ж����ݿ��Ƿ����
	{
		tmp_path = cp.GetRootPath() + db_name;
		_mkdir(tmp_path.c_str());
		return true;
	}
	else
	{
		std::cout << "���ݿ��Ѿ�����" << std::endl;
		return false;
	}
}

bool DropDatabase(std::string db_name, CatalogPosition &cp)
{
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //�ж����ݿ��Ƿ����
	{
		std::cout << "���ݿⲻ����" << std::endl;
		return false;
	}
	else
	{
		tmp_path = cp.GetRootPath() + db_name;
		// ɾ��Ŀ¼���ļ�
		auto t = tmp_path + "/";
		DelFilesInFolder(t);
		// ɾ��Ŀ¼
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
		//�ж��Ƿ�����Ŀ¼  
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//���������Ҫ  
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				std::string newPath = folderPath + FileInfo.name;
				newPath += "/";
				DelFilesInFolder(newPath);
				// ɾ�����ļ���
				_rmdir(newPath.c_str());
			}
		}
		else
		{
			//fout << folderPath << FileInfo.name << " ";
			// ɾ���ļ�
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
		// �������ͨ�ļ��������
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
	// ���ж����ݿ��Ƿ����
	std::string tmp_path = cp.GetRootPath() + db_name;

	if (_access(tmp_path.c_str(), 0) == -1)  //�ж����ݿ��Ƿ����
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
	// TODO ��鴴����Ϣ�Լ���ǰĿ¼�Ƿ������ݿ���

	// ��ȡ����
	std::string table_name = tb_create_info.table_name;

	// ��ȡ�ؼ���λ��
	int KeyTypeIndex = 0;
	for (int j = 0; j < tb_create_info.columns_info.size(); j++)
	{
		if (tb_create_info.columns_info[j].isPrimary)
		{
			KeyTypeIndex = j;
			break;
		}
	}

	//��ȡ�ֶ���Ϣ
	char RecordTypeInfo[RecordColumnCount];          // ��¼�ֶ�������Ϣ
	char *ptype = RecordTypeInfo;
	char RecordColumnName[RecordColumnCount / 4 * ColumnNameLength];
	char*pname = RecordColumnName;

	auto &column_info_ref = tb_create_info.columns_info;
	for (int i = 0; i < column_info_ref.size(); i++)
	{
		// ������Ϣ
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
		// ������Ϣ
		strcpy(pname, column_info_ref[i].name.c_str());
		pname += ColumnNameLength;
	}
	*ptype = '\0';

	// ���������ļ�
	std::string idx_file = path + table_name + ".idx";
	BTree tree(idx_file, KeyTypeIndex, RecordTypeInfo, RecordColumnName);
	// ���������ļ�
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

	if (_access(idx.c_str(), 0) == -1|| _access(dbf.c_str(), 0) == -1)  //�жϱ��Ƿ����
	{
		return false;
	}
	else
	{
		// ɾ�����ļ�
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
		// �������ͨ�ļ��������
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
	if (!GetCp().GetIsInSpeDb())  // ������ھ������ݿ�Ŀ¼�£����ܲ����¼
		return false;

	std::string idx_file = path + tb_insert_info.table_name + ".idx";
	std::string dbf_file = path + tb_insert_info.table_name + ".dbf";
	BTree tree(idx_file);
	auto phead = tree.GetPtrIndexHeadNode();

	KeyAttr key;

#ifndef NDEBUG
	std::cout << "��ӡ����ͷ����Ϣ" << std::endl;
	std::cout << "��¼�ֶ����ͣ�" << phead->RecordTypeInfo << std::endl;
#endif

	int sz_col = 0;// �ֶθ���
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		if (phead->RecordTypeInfo[i] == 'I' || phead->RecordTypeInfo[i] == 'C' || phead->RecordTypeInfo[i] == 'D')sz_col++;
	}

#ifndef NDEBUG
	std::cout << "�����ֶ����ƣ�" << std::endl;
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		std::cout << pColumnName << std::endl;
		pColumnName += ColumnNameLength;
	}
#endif

	// ����¼��Ϣ��װ�ɼ�¼���ݶ���
	RecordHead record_head;
	int column_id = 0;
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
	{
		Column_Cell cc;
		if (phead->RecordTypeInfo[i] == 'I')
		{
			//�ҵ���Ӧ���ֶ�����
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//�ڲ����¼��Ѱ�Ҹ��ֶε�ֵ
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
				// Ĭ��ֵ���
				cc.column_type = Column_Type::I;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'D')
		{
			//�ҵ���Ӧ���ֶ�����
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//�ڲ����¼��Ѱ�Ҹ��ֶε�ֵ
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
				// Ĭ��ֵ���
				cc.column_type = Column_Type::D;
			}

			column_id++;
			record_head.AddColumnCell(cc);
		}

		if (phead->RecordTypeInfo[i] == 'C')
		{
			//�ҵ���Ӧ���ֶ�����
			char *pColumnName = phead->RecordColumnName + column_id*ColumnNameLength;
			//�ڲ����¼��Ѱ�Ҹ��ֶε�ֵ
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
				char*pChar = (char*)malloc(sz + 3); // �����������ֽ����������û�������ַ�������ֵ
				memcpy(pChar, &(phead->RecordTypeInfo[i + 1]), 3);
				pChar += 3;
				strcpy(pChar, tb_insert_info.insert_info[k].column_value.c_str());
				cc.column_value.StrValue = pChar - 3;
			}
			else
			{
				// Ĭ��ֵ���
				cc.column_type = Column_Type::C;
				cc.column_value.StrValue = nullptr;
			}
			column_id++;
			record_head.AddColumnCell(cc);
		}

	}

	// ���������ļ�
	Record record;
	auto fd = record.InsertRecord(dbf_file, record_head);
	// ��������
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

SelectPrintInfo SelectTable(TB_Select_Info tb_select_info, std::string path /*= std::string("./")*/)
{
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;

	for (int i = 0; i < tb_select_info.vec_cmp_cell.size(); i++)
	{
		// �������㵥���ֶεļ�¼
		fds = Search(tb_select_info.vec_cmp_cell[i], tb_select_info.table_name, GetCp().GetCurrentPath());
		// �µĽ����֮ǰ�Ľ���󽻼�
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
	//fds = RangeSearch(tb_select_info.vec_cmp_cell[0], tb_select_info.table_name, GetCp().GetCurrentPath());



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
	// ��ȡ��Ӧ�ֶεĳ���
	std::vector<std::string> col_name;
	std::vector<int> col_len;
	auto phead = tree.GetPtrIndexHeadNode();
	int sz_col = 0; // �ֶθ���
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
	std::cout << "�����ֶ����ƣ�" << std::endl;
	char *pColumnName = phead->RecordColumnName;
	for (int j = 0; j < sz_col; j++)
	{
		//std::cout << pColumnName << std::endl;
		col_name.push_back(pColumnName);
		pColumnName += ColumnNameLength;
	}





	// �����²�����expr������װ�ɲ�������
	std::vector<CompareCell> cmp_cells;
	auto fields_name = GetColumnAndTypeFromTable(tb_update_info.table_name, GetCp().GetCurrentPath());

	for (int i = 0; i < tb_update_info.expr.size(); i++)
	{
		CompareCell cmp_cell = CreateCmpCell(tb_update_info.expr[i].field, GetType(tb_update_info.expr[i].field, fields_name)
			, GetOperatorType(tb_update_info.expr[i].op), tb_update_info.expr[i].value);
		cmp_cells.push_back(cmp_cell);
	}
	
	// ������������������ֶ�
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	std::vector<std::pair<KeyAttr, FileAddr>> fds;
	// ����
	for (int i = 0; i < cmp_cells.size(); i++)
	{
		// �������㵥���ֶεļ�¼
		fds = Search(cmp_cells[i], tb_update_info.table_name, GetCp().GetCurrentPath());
		// �µĽ����֮ǰ�Ľ���󽻼�
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

	// ���¼�¼
	for (int i = 0; i < res.size(); i++)
	{
		// �ȶ����ɵļ�¼
		auto record_head = GetDbfRecord(tb_update_info.table_name, res[i].second, path);
		//����ÿ��Ҫ���µ��ֶ�ֵ
		
		for (int j = 0; j < tb_update_info.field_value.size(); j++)
		{
			auto head = record_head.GetFirstColumn();
			std::string new_str;
			int n = 0;
			while (head)
			{
				if (head->columu_name == tb_update_info.field_value[j].field)
				{
					// ����
					switch (head->column_type)
					{
					case Column_Type::I:
						head->column_value.IntValue = stoi(tb_update_info.field_value[i].value);
						break;
					case Column_Type::D:
						head->column_value.DoubleValue = stod(tb_update_info.field_value[i].value);
						break;
					case Column_Type::C:
						// ��ȡ����
						
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
						break;
					default:
						break;
					}
					
					break;
				}
				head = head->next;
			}
		}

		//д��
		Record record;
		record.UpdateRecord(file_dbf, record_head, res[i].second);
		
		// �ж�������ֵ��û�б�����
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
		// �޸���������
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
	// ��ȡ����ڴ��ַ
	char* pRecTypeInfo = tree.GetPtrIndexHeadNode()->RecordTypeInfo;
	//std::cout << pRecTypeInfo << std::endl;
	auto pdata = (char*)GetGlobalFileBuffer()[dbf_file.c_str()]->ReadRecord(&fd);
	pdata += sizeof(FileAddr);  // ÿ����¼ͷ��Ĭ�����Ӹü�¼�ĵ�ֵַ

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
			// ��ȡ�ַ�������
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

	// ��¼�����ֶ�����
	std::vector<Column_Type> tb_types;
	int sz_col = 0;// �ֶθ���
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

	// ��¼�����ֶ�����
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
	// ������ҽ��
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// �����ļ���
	std::string file_idx = path + table_name + ".idx";

	// ��ȡ���� ��Ϣ
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// �жϴ����ҵ��ֶ��Ƿ��������ֶ�
	bool bKeyComumn = false;
	int sz_col = 0;// �ֶθ���
	for (int i = 0; phead->RecordTypeInfo[i] != '\0'; i++)
		if (phead->RecordTypeInfo[i] == 'I' || phead->RecordTypeInfo[i] == 'C' || phead->RecordTypeInfo[i] == 'D')sz_col++;
	int key_index = phead->KeyTypeIndex;
	char *pColumnName = phead->RecordColumnName;
	for(int i=0; i<key_index;i++)
		pColumnName += ColumnNameLength;


	if (compare_cell.cmp_value.columu_name == pColumnName)
	{
		bKeyComumn = true;

	}
		
	// ����

	if (bKeyComumn)
	{
#ifndef NDEBUG
		std::cout << "��������" << std::endl;
#endif
		res = KeySearch(compare_cell, table_name, path);
	}
	else
	{
#ifndef NDEBUG
		std::cout << "��������" << std::endl;
#endif
		res = RangeSearch(compare_cell, table_name, path);
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> KeySearch(CompareCell compare_cell, std::string table_name, std::string path /*= std::string("./")*/)
{
	// ������ҽ��
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// �����ļ���
	std::string file_idx = path + table_name + ".idx";

	// ��ȡ���� ��Ϣ
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// ����ǲ�����ȵ�ֵ
	if (compare_cell.OperType == Operator_Type::E)
	{
#ifndef NDEBUG
		std::cout << "����B+������" << std::endl;
#endif
		auto fd = tree.Search(compare_cell.cmp_value);
		if (fd.offSet!=0)
		{
			res.push_back({ compare_cell.cmp_value , fd });
		}
	}
	else  // ���Ż�::�ؼ��ֶ��ֲ���
	{
#ifndef NDEBUG
		std::cout << "����������������" << std::endl;
#endif
		// ��һ�����ݽ���ַ
		auto node_fd = phead->MostLeftNode;

		while (node_fd.offSet != 0)
		{
			//ȡ������ڼ�¼
			auto pNode = tree.FileAddrToMemPtr(node_fd);
			for (int i = 0; i < pNode->count_valid_key; i++)
			{
				// ���ұȽϵ��ֶ�
				Column_Cell cc(pNode->key[i]);
				bool isSearched = compare_cell(cc);
				if (isSearched)  // ��������
				{
					res.push_back({ pNode->key[i] ,pNode->children[i] });
				}
			}
			// ��һ�����ݽ��
			node_fd = tree.FileAddrToMemPtr(node_fd)->next;
		}
	}

	return res;
}

std::vector<std::pair<KeyAttr, FileAddr>> RangeSearch(CompareCell compare_cell, std::string table_name, std::string path)
{
	// ������ҽ��
	std::vector<std::pair<KeyAttr, FileAddr>> res;
	// �����ļ���
	std::string file_idx = path + table_name + ".idx";

	// ��ȡ���� ��Ϣ
	BTree tree(file_idx);
	auto phead = tree.GetPtrIndexHeadNode();

	// ��һ�����ݽ���ַ
	auto node_fd = phead->MostLeftNode;

	while (node_fd.offSet!=0)
	{
		//ȡ������ڼ�¼
		auto pNode = tree.FileAddrToMemPtr(node_fd);
		for (int i = 0; i < pNode->count_valid_key; i++)
		{
			RecordHead record = GetDbfRecord(table_name, pNode->children[i], path);

			// ���ұȽϵ��ֶ�
			auto pColumn = record.GetFirstColumn();
			while (pColumn && pColumn->columu_name != compare_cell.cmp_value.columu_name)pColumn = pColumn->next;
			bool isSearched = compare_cell(*pColumn);
			if (isSearched)  // ��������
			{
				res.push_back({ pNode->key[i] ,pNode->children[i] });
			}
		}
		// ��һ�����ݽ��
		node_fd = tree.FileAddrToMemPtr(node_fd)->next;
	}

	return res;
}
