/*
void remux(int fin, int fout, int pack_size, int mult)
{
	Remux rem;
	uint8_t buf[MAX_PACK_L];
	int r = 0;
	int i;
	uint8_t mpeg_end[4] = { 0x00, 0x00, 0x01, 0xB9 };
	int data_size;

	rem.pack_size = pack_size;
	data_size = pack_size - MAX_H_SIZE;
	refill_buffy(&rem);
	
	if ( get_video_info(&rem) < 0 ){
		fprintf(stderr,"ERROR: Can't find valid video stream\n");
		exit(1);
	}

	i = 0;
	while(! rem.time_off && i < rem.vframen) {
		if( (rem.time_off = rem.vframe_list[i].time)) break;
		i++;
	}

	if ( get_audio_info(&rem) < 0 ){
		fprintf(stderr,"ERROR: Can't find valid audio stream\n");
		exit(1);
	}
	
	rem.vpts = rem.vpts_list[0].PTS;
	rem.vdts = rem.vpts;
	rem.vpts_off = rem.vpts;
	rem.apts = rem.apts_list[0].PTS;
	rem.apts_off = rem.apts;
	ptsdiff = rem.vpts - rem.apts;
	if (ptsdiff > 0) rem.vpts_off -= ptsdiff;
	else rem.apts_off -= -ptsdiff;

	rem.muxr = (rem.video_info.bit_rate + 
		    rem.audio_info.bit_rate)/400;
	SCR_inc = 1800 * pack_size / rem.muxr;
	
	r = 0;
	while ( rem.vptsn < 2 && !r) r = refill_buffy(&rem);
	r = 0;
	while ( rem.aptsn < 2 && !r) r = refill_buffy(&rem);

	rem.vpts_delay =  (uint32_t)(2*90000ULL* (uint64_t)pack_size/rem.muxr);
	rem.vpts_delay = rem.dts_delay;
	rem.apts_delay = rem.vpts_delay;

	vbuf_max = 29440;
	abuf_max = 4096;
	vbuf = 0;
	abuf = 0;
	pos = write_ps_header(buf,rem.SCR,rem.muxr, 1, 0, 0, 1, 1, 1, 
			      0xC0, 0, 32, 0xE0, 1, 230);
	pos += write_pes_header( PADDING_STREAM, pack_size-pos, 0, buf+pos,0);
	pos = rem.pack_size;
	write( fout, buf, pos);

	apos = rem.aread;
	vpos = rem.vread;

	while( ring_rest(&rem.aud_buffy) && ring_rest(&rem.vid_buffy) ){
		uint32_t next_apts;
		uint32_t next_vdts;
		int asize, vsize;

		r1 = 0;
		r2 = 0;
		while ( rem.aframen < 2 && !r1) 
			r1 = refill_buffy(&rem);
		while ( rem.vframen < 2 && !r2) 
			r2 = refill_buffy(&rem);
		if (r1 && r2) break;

		if ( !r1 && apos <= rem.aread)
			apos = rem.aframe_list[1].pos;
		if ( !r2 && vpos <= rem.vread)
			vpos = rem.vframe_list[1].pos;
		apack_size = apos - rem.aread; 
		vpack_size = vpos - rem.vread; 
		

		next_vdts = (uint32_t)((uint64_t)rem.vdts + rem.vpts_delay 
				  - rem.vpts_off) ;
		ok_video = ( rem.SCR < next_vdts);

		next_apts = (uint32_t)((uint64_t)rem.apts + rem.apts_delay 
				  - rem.apts_off) ;
		ok_audio = ( rem.SCR  < next_apts);

		asize = (apack_size > data_size ? data_size: apack_size);
		vsize = (vpack_size > data_size ? data_size: vpack_size);


		if( vbuf+vsize  < vbuf_max && vsize && ok_audio ){
			pos = write_video_pes( &rem, buf, &vpack_size);
			write( fout, buf, pos);
			vbuf += vpack_size;
			vbufn = add_pts( vbufl, rem.vdts, vpack_size, 
					 0, vbufn, 0);
			packets++;
		} else if ( abuf+asize < abuf_max && asize &&
			    ok_video  ){
			pos = write_audio_pes( &rem, buf, &apack_size);
			write( fout, buf, pos);
			abuf += apack_size;
			abufn = add_pts( abufl, rem.apts, apack_size, 
					 0, abufn, 0);
			packets++;
		} else if ( abuf+asize < abuf_max && asize &&
			    !ok_audio){
			pos = write_audio_pes( &rem, buf, &apack_size);
			write( fout, buf, pos);
			abuf += apack_size;
			abufn = add_pts( abufl, rem.apts, apack_size, 
					 0, abufn, 0);
			packets++;
		} else if (vbuf+vsize  < vbuf_max && vsize &&
			   !ok_video){
			pos = write_video_pes( &rem, buf, &vpack_size);
			write( fout, buf, pos);
			vbuf += vpack_size;
			vbufn = add_pts( vbufl, rem.vdts, vpack_size, 
					 0, vbufn, 0);
			packets++;
		} else {
			pos = write_ps_header(buf,rem.SCR,rem.muxr, 1, 0, 0, 
					      1, 1, 1, 0, 0, 0, 0, 0, 0);

			pos += write_pes_header( PADDING_STREAM, pack_size-pos,
						 0, buf+pos, 0);
			write( fout, buf, pos);
		}


		if (rem.SCR > rem.vdts+rem.vpts_off -rem.vpts_delay) 
			rem.SCR = rem.vdts-rem.vpts_off;
		rem.SCR = (uint32_t)((uint64_t) rem.SCR + SCR_inc);

		if ( rem.apts_off + rem.SCR < rem.apts_delay ) pts_d = 0;
		else pts_d = (uint64_t) rem.SCR + rem.apts_off - rem.apts_delay;
		abuf -= del_ptss( abufl, (uint32_t) pts_d, &abufn);

		if ( rem.vpts_off + rem.SCR < rem.vpts_delay ) pts_d = 0;
		else pts_d = (uint64_t) rem.SCR + rem.vpts_off - rem.vpts_delay;
		vbuf -= del_ptss( vbufl, (uint32_t) pts_d, &vbufn);

	}
	pos = write_ps_header(buf,rem.SCR,rem.muxr, 1, 0, 0, 1, 1, 1, 
			      0, 0, 0, 0, 0, 0);

	pos += write_pes_header( PADDING_STREAM, pack_size-pos-4, 0, 
				 buf+pos, 0);
	pos = rem.pack_size-4;
	write( fout, buf, pos);

	write( fout, mpeg_end, 4);
}

*/
